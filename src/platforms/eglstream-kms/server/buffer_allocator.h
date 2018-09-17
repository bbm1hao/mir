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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#ifndef MIR_PLATFORMS_EGLSTREAM_BUFFER_ALLOCATOR_
#define MIR_PLATFORMS_EGLSTREAM_BUFFER_ALLOCATOR_

#include "mir/graphics/graphic_buffer_allocator.h"
#include "mir/graphics/wayland_allocator.h"
#include "mir/graphics/buffer_id.h"
#include "mir/graphics/egl_extensions.h"

#include "wayland-eglstream-controller.h"

#include <memory>

namespace mir
{
namespace graphics
{

namespace eglstream
{

class BufferAllocator:
    public graphics::GraphicBufferAllocator,
    public graphics::WaylandAllocator
{
public:
    BufferAllocator(EGLDisplay dpy, EGLContext shared_context);

    std::shared_ptr<Buffer> alloc_buffer(graphics::BufferProperties const& buffer_properties) override;

    std::shared_ptr<Buffer> alloc_software_buffer(geometry::Size size, MirPixelFormat format) override;
    std::shared_ptr<Buffer> alloc_buffer(
        geometry::Size size, uint32_t native_format, uint32_t native_flags) override;

    std::vector<MirPixelFormat> supported_pixel_formats() override;

    void bind_display(wl_display* display) override;
    std::shared_ptr<Buffer> buffer_from_resource(
        wl_resource* buffer,
        std::function<void()>&& on_consumed,
        std::function<void()>&& on_release) override;

private:
    static void create_buffer_eglstream_resource(
        wl_client* client,
        wl_resource* eglstream_controller,
        wl_resource* surface,
        wl_resource* buffer);
    static void bind_eglstream_controller(
        wl_client* client,
        void* ctx,
        uint32_t version,
        uint32_t id);

    EGLExtensions::WaylandExtensions const extensions;
    EGLExtensions::NVStreamAttribExtensions const nv_extensions;
    EGLDisplay dpy;
    EGLContext ctx;
    static struct wl_eglstream_controller_interface const impl;
};

}
}
}

#endif // MIR_PLATFORMS_EGLSTREAM_BUFFER_ALLOCATOR_
