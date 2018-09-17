/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include <epoxy/egl.h>

#include "buffer_allocator.h"
#include "buffer_texture_binder.h"
#include "mir/anonymous_shm_file.h"
#include "shm_buffer.h"
#include "mir/graphics/buffer_properties.h"
#include "software_buffer.h"
#include "wayland-eglstream-controller.h"
#include "mir/graphics/egl_error.h"
#include "mir/graphics/texture.h"

#define MIR_LOG_COMPONENT "platform-eglstream-kms"
#include "mir/log.h"

#include <wayland-server-core.h>

#include <mutex>

#include <boost/throw_exception.hpp>
#include <boost/exception/errinfo_errno.hpp>

#include <system_error>
#include <cassert>


namespace mg  = mir::graphics;
namespace mge = mg::eglstream;
namespace mgc = mg::common;
namespace geom = mir::geometry;

#ifndef EGL_WL_wayland_eglstream
#define EGL_WL_wayland_eglstream 1
#define EGL_WAYLAND_EGLSTREAM_WL              0x334B
#endif /* EGL_WL_wayland_eglstream */

namespace
{
EGLConfig choose_config(EGLDisplay display)
{
    EGLint const config_attr[] = {
        EGL_SURFACE_TYPE, EGL_STREAM_BIT_KHR,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, MIR_SERVER_EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint num_egl_configs;
    EGLConfig egl_config;
    if (eglChooseConfig(display, config_attr, &egl_config, 1, &num_egl_configs) != EGL_TRUE)
    {
        BOOST_THROW_EXCEPTION(mg::egl_error("Failed to chose EGL config"));
    } else if (num_egl_configs != 1)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error{"Failed to find compatible EGL config"});
    }

    return egl_config;
}

EGLContext create_context(EGLDisplay display, EGLContext shared_context)
{
    eglBindAPI(MIR_SERVER_EGL_OPENGL_API);

    EGLint const context_attr[] = {
#if MIR_SERVER_EGL_OPENGL_BIT == EGL_OPENGL_ES2_BIT
        EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
        EGL_NONE
    };

    EGLContext context =
        eglCreateContext(display, choose_config(display), shared_context, context_attr);
    if (context == EGL_NO_CONTEXT)
    {
        BOOST_THROW_EXCEPTION(mg::egl_error("Failed to create EGL context"));
    }

    return context;
}
}

mge::BufferAllocator::BufferAllocator(EGLDisplay dpy, EGLContext shared_context)
    : dpy{dpy},
      ctx{create_context(dpy, shared_context)}
{
}

std::shared_ptr<mg::Buffer> mge::BufferAllocator::alloc_buffer(
    BufferProperties const& buffer_properties)
{
    if (buffer_properties.usage == mg::BufferUsage::software)
        return alloc_software_buffer(buffer_properties.size, buffer_properties.format);
    BOOST_THROW_EXCEPTION(std::runtime_error("platform incapable of creating hardware buffers"));
}

std::shared_ptr<mg::Buffer> mge::BufferAllocator::alloc_software_buffer(geom::Size size, MirPixelFormat format)
{
    if (!mgc::ShmBuffer::supports(format))
    {
        BOOST_THROW_EXCEPTION(
            std::runtime_error(
                "Trying to create SHM buffer with unsupported pixel format"));
    }

    auto const stride = geom::Stride{ MIR_BYTES_PER_PIXEL(format) * size.width.as_uint32_t() };
    size_t const size_in_bytes = stride.as_int() * size.height.as_int();
    return std::make_shared<mge::SoftwareBuffer>(
        std::make_unique<mir::AnonymousShmFile>(size_in_bytes), size, format);
}

std::vector<MirPixelFormat> mge::BufferAllocator::supported_pixel_formats()
{
    // Lazy
    return {mir_pixel_format_argb_8888, mir_pixel_format_xrgb_8888};
}

std::shared_ptr<mg::Buffer> mge::BufferAllocator::alloc_buffer(geometry::Size, uint32_t, uint32_t)
{
    BOOST_THROW_EXCEPTION(std::runtime_error("platform incapable of creating buffers"));
}

namespace
{

GLuint gen_texture_handle()
{
    GLuint tex;
    glGenTextures(1, &tex);
    return tex;
}

struct EGLStreamTextureConsumer
{
    EGLStreamTextureConsumer(EGLDisplay dpy, EGLStreamKHR&& stream)
        : dpy{dpy},
          stream{std::move(stream)},
          texture{gen_texture_handle()}
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

        if (eglStreamConsumerGLTextureExternalKHR(dpy, this->stream) != EGL_TRUE)
        {
            BOOST_THROW_EXCEPTION(
                mg::egl_error("Failed to bind client EGLStream to a texture consumer"));
        }
    }

    ~EGLStreamTextureConsumer()
    {
        eglDestroyStreamKHR(dpy, stream);
        glDeleteTextures(1, &texture);
    }

    EGLDisplay const dpy;
    EGLStreamKHR const stream;
    GLuint const texture;
};

struct Sync
{
    /*
     * The reserve_sync/set_consumer_sync dance is magical!
     */
    void reserve_sync()
    {
        sync_mutex.lock();
    }

    void set_consumer_sync(GLsync syncpoint)
    {
        if (sync)
        {
            glDeleteSync(sync);
        }
        sync = syncpoint;
        sync_mutex.unlock();
    }

    std::mutex sync_mutex;
    GLsync sync{nullptr};
};

struct BoundEGLStream
{
    static void associate_stream(wl_resource* buffer, EGLDisplay dpy, EGLStreamKHR stream)
    {
        BoundEGLStream* me;
        if (auto notifier = wl_resource_get_destroy_listener(buffer, &on_buffer_destroyed))
        {
            /* We're associating a buffer which has an existing stream with a new stream?
             * The protocol is unclear whether this is an expected behaviour, but the obvious
             * thing to do is to destroy the old stream and associate the new one.
             */
            me = wl_container_of(notifier, me, destruction_listener);
        }
        else
        {
            me = new BoundEGLStream;
            me->destruction_listener.notify = &on_buffer_destroyed;
            wl_resource_add_destroy_listener(buffer, &me->destruction_listener);
        }

        me->producer = std::make_shared<EGLStreamTextureConsumer>(dpy, std::move(stream));
    }

    class TextureHandle
    {
    public:
        void bind()
        {
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, provider->texture);
        }

        void reserve_sync()
        {
            sync->reserve_sync();
        }

        void set_consumer_sync(GLsync sync)
        {
            this->sync->set_consumer_sync(sync);
        }

        TextureHandle(TextureHandle&&) = default;
    private:
        friend class BoundEGLStream;

        TextureHandle(
            std::shared_ptr<Sync> syncpoint,
            std::shared_ptr<EGLStreamTextureConsumer const> producer)
            : sync{std::move(syncpoint)},
              provider{std::move(producer)}
        {
            bind();
            /*
             * Isn't this a fun dance!
             *
             * We must insert a glWaitSync here to ensure that the texture is not
             * modified while the commands from the render thread are still executing.
             *
             * We need to lock until *after* eglStreamConsumerAcquireKHR because,
             * once that has executed, it's guaranteed that glBindTexture() in the
             * render thread will bind the new texture (ie: there's some implicit syncpoint
             * action happening)
             */
            std::lock_guard<std::mutex> lock{sync->sync_mutex};
            if (sync->sync)
            {
                mir::log_debug("Inserting glWaitSync");
                glWaitSync(sync->sync, 0, GL_TIMEOUT_IGNORED);
                sync->sync = nullptr;
            }
            if (eglStreamConsumerAcquireKHR(provider->dpy, provider->stream) != EGL_TRUE)
            {
                BOOST_THROW_EXCEPTION(
                    mg::egl_error("Failed to latch texture from client EGLStream"));
            }
        }

        TextureHandle(TextureHandle const&) = delete;
        TextureHandle& operator=(TextureHandle const&) = delete;

        std::shared_ptr<Sync> const sync;
        std::shared_ptr<EGLStreamTextureConsumer const> provider;
    };

    static TextureHandle texture_for_buffer(wl_resource* buffer)
    {
        if (auto notifier = wl_resource_get_destroy_listener(buffer, &on_buffer_destroyed))
        {
            BoundEGLStream* me;
            me = wl_container_of(notifier, me, destruction_listener);
            return TextureHandle{me->consumer_sync, me->producer};
        }
        BOOST_THROW_EXCEPTION((std::runtime_error{"Buffer does not have an associated EGLStream"}));
    }
private:
    static void on_buffer_destroyed(wl_listener* listener, void*)
    {
        static_assert(
            std::is_standard_layout<BoundEGLStream>::value,
            "BoundEGLStream must be Standard Layout for wl_container_of to be defined behaviour");

        BoundEGLStream* me;
        me = wl_container_of(listener, me, destruction_listener);
        delete me;
    }

    std::shared_ptr<Sync> const consumer_sync{std::make_shared<Sync>()};
    std::shared_ptr<EGLStreamTextureConsumer const> producer;
    wl_listener destruction_listener;
};
}

void mge::BufferAllocator::create_buffer_eglstream_resource(
    wl_client* /*client*/,
    wl_resource* eglstream_controller_resource,
    wl_resource* /*surface*/,
    wl_resource* buffer)
{
    auto const allocator = static_cast<mge::BufferAllocator*>(
        wl_resource_get_user_data(eglstream_controller_resource));

    eglMakeCurrent(allocator->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, allocator->ctx);

    EGLAttrib const attribs[] = {
        EGL_WAYLAND_EGLSTREAM_WL, (EGLAttrib)buffer,
        EGL_NONE
    };

    auto stream = allocator->nv_extensions.eglCreateStreamAttribNV(allocator->dpy, attribs);

    if (stream == EGL_NO_STREAM_KHR)
    {
        BOOST_THROW_EXCEPTION((mg::egl_error("Failed to create EGLStream from Wayland buffer")));
    }

    BoundEGLStream::associate_stream(buffer, allocator->dpy, stream);
}

struct wl_eglstream_controller_interface const mge::BufferAllocator::impl{
    create_buffer_eglstream_resource
};

void mge::BufferAllocator::bind_eglstream_controller(
    wl_client* client,
    void* ctx,
    uint32_t version,
    uint32_t id)
{
    auto resource = wl_resource_create(client, &wl_eglstream_controller_interface, version, id);

    if (resource == nullptr)
    {
        wl_client_post_no_memory(client);
        mir::log_warning("Failed to create client eglstream-controller resource");
        return;
    }

    wl_resource_set_implementation(
        resource,
        &impl,
        ctx,
        nullptr);
}


void mir::graphics::eglstream::BufferAllocator::bind_display(wl_display* display)
{
    if (!wl_global_create(
        display,
        &wl_eglstream_controller_interface,
        1,
        this,
        &bind_eglstream_controller))
    {
        BOOST_THROW_EXCEPTION((std::runtime_error{"Failed to publish wayland-eglstream-controller global"}));
    }

    dpy = eglGetCurrentDisplay();

    if (dpy == EGL_NO_DISPLAY)
    {
        BOOST_THROW_EXCEPTION((
            std::logic_error{"WaylandAllocator::bind_display called without an active EGL Display"}));
    }
    if (extensions.eglBindWaylandDisplayWL(dpy, display) != EGL_TRUE)
    {
        BOOST_THROW_EXCEPTION((mg::egl_error("Failed to bind Wayland EGL display")));
    }

    std::vector<char const*> missing_extensions;
    for (char const* extension : {
        "EGL_KHR_stream_consumer_gltexture",
        "EGL_NV_stream_attrib"})
    {
        if (!epoxy_has_egl_extension(dpy, extension))
        {
            missing_extensions.push_back(extension);
        }
    }

    if (!missing_extensions.empty())
    {
        std::stringstream message;
        message << "Missing required extension" << (missing_extensions.size() > 1 ? "s:" : ":");
        for (auto missing_extension : missing_extensions)
        {
            message << " " << missing_extension;
        }

        BOOST_THROW_EXCEPTION((std::runtime_error{message.str()}));
    }

    mir::log_info("Bound EGLStreams-backed WaylandAllocator display");
}

namespace
{
class EGLStreamBuffer :
    public mg::BufferBasic,
    public mg::NativeBufferBase,
    public mg::gl::Texture
{
public:
    EGLStreamBuffer(
        BoundEGLStream::TextureHandle tex,
        std::function<void()>&& on_consumed,
        MirPixelFormat format,
        geom::Size size)
        : size_{size},
          format{format},
          tex{std::move(tex)},
          on_consumed{std::move(on_consumed)}
    {
    }

    std::shared_ptr<mir::graphics::NativeBuffer> native_buffer_handle() const override
    {
        return nullptr;
    }

    mir::geometry::Size size() const override
    {
        return size_;
    }

    MirPixelFormat pixel_format() const override
    {
        return format;
    }

    NativeBufferBase* native_buffer_base() override
    {
        return this;
    }

    mg::gl::Texture::Target target() const override
    {
        return Target::External;
    }

    void bind() override
    {
        tex.reserve_sync();
        tex.bind();
    }

    void add_syncpoint() override
    {
        tex.set_consumer_sync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
        // TODO: We're going to flush an *awful* lot; try and work out a way to batch this.
        glFlush();
        on_consumed();
    }

private:
    mir::geometry::Size const size_;
    MirPixelFormat const format;
    BoundEGLStream::TextureHandle tex;
    std::function<void()> on_consumed;
};
}

std::shared_ptr<mir::graphics::Buffer>
mir::graphics::eglstream::BufferAllocator::buffer_from_resource(
    wl_resource* buffer,
    std::function<void()>&& on_consumed,
    std::function<void()>&& /*on_release*/)
{
    EGLint width, height/*, format*/;
    if (extensions.eglQueryWaylandBufferWL(dpy, buffer, EGL_WIDTH, &width) != EGL_TRUE)
    {
        BOOST_THROW_EXCEPTION(mg::egl_error("Failed to query Wayland buffer width"));
    }
    if (extensions.eglQueryWaylandBufferWL(dpy, buffer, EGL_HEIGHT, &height) != EGL_TRUE)
    {
        BOOST_THROW_EXCEPTION(mg::egl_error("Failed to query Wayland buffer height"));
    }
/*    if (extensions.eglQueryWaylandBufferWL(dpy, buffer, EGL_TEXTURE_FORMAT, &format) != EGL_TRUE)
    {
        BOOST_THROW_EXCEPTION(mg::egl_error("Failed to query Wayland buffer format"));
    }

    auto const mir_format =
        [format]()
        {
            switch(format)
            {
            case EGL_TEXTURE_RGB:
                return mir_pixel_format_xrgb_8888;
            case EGL_TEXTURE_RGBA:
                return mir_pixel_format_argb_8888;
            default:
                BOOST_THROW_EXCEPTION((std::invalid_argument{"YUV buffers are unimplemented"}));
            }
        }();
*/
    auto const result = std::make_shared<EGLStreamBuffer>(
        BoundEGLStream::texture_for_buffer(buffer),
        std::move(on_consumed),
        mir_pixel_format_argb_8888,
        geom::Size{width, height});

    // We've latched the texture, so the buffer is now safe to release.
//    auto release = std::move(on_release);
//    release();

    return result;
}

