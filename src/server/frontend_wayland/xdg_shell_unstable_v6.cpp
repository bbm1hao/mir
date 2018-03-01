/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#include "xdg_shell_unstable_v6.h"

#include "wayland_utils.h"
#include "basic_surface_event_sink.h"
#include "wl_seat.h"
#include "wl_surface.h"
#include "wl_mir_window.h"

#include "mir/scene/surface_creation_parameters.h"
#include "mir/frontend/shell.h"
#include "mir/optional_value.h"

namespace mf = mir::frontend;
namespace geom = mir::geometry;

namespace mir
{
namespace frontend
{

class Shell;
class XdgSurfaceUnstableV6;
class WlSeat;
class XdgSurfaceUnstableV6EventSink;

class XdgSurfaceUnstableV6 : wayland::XdgSurfaceV6, WlAbstractMirWindow
{
public:
    XdgSurfaceUnstableV6* get_xdgsurface(wl_resource* surface) const;

    XdgSurfaceUnstableV6(wl_client* client, wl_resource* parent, uint32_t id, wl_resource* surface,
                 std::shared_ptr<Shell> const& shell, WlSeat& seat);
    ~XdgSurfaceUnstableV6() override;

    void destroy() override;
    void get_toplevel(uint32_t id) override;
    void get_popup(uint32_t id, struct wl_resource* parent, struct wl_resource* positioner) override;
    void set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height) override;
    void ack_configure(uint32_t serial) override;

    void set_parent(optional_value<SurfaceId> parent_id);
    void set_title(std::string const& title);
    void move(struct wl_resource* seat, uint32_t serial);
    void resize(struct wl_resource* /*seat*/, uint32_t /*serial*/, uint32_t edges);
    void set_notify_resize(std::function<void(geometry::Size const& new_size)> notify_resize);
    void set_max_size(int32_t width, int32_t height);
    void set_min_size(int32_t width, int32_t height);
    void set_maximized();
    void unset_maximized();

    using WlAbstractMirWindow::client;
    using WlAbstractMirWindow::params;
    using WlAbstractMirWindow::surface_id;

    struct wl_resource* const parent;
    std::shared_ptr<Shell> const shell;
    std::shared_ptr<XdgSurfaceUnstableV6EventSink> const sink;
};

class XdgSurfaceUnstableV6EventSink : public BasicSurfaceEventSink
{
public:
    using BasicSurfaceEventSink::BasicSurfaceEventSink;

    XdgSurfaceUnstableV6EventSink(WlSeat* seat, wl_client* client, wl_resource* target, wl_resource* event_sink,
                          std::shared_ptr<bool> const& destroyed);

    void send_resize(geometry::Size const& new_size) const override;

    std::function<void(geometry::Size const& new_size)> notify_resize = [](auto){};

private:
    void post_configure(int serial) const;

    std::shared_ptr<bool> const destroyed;
};

class XdgPopupUnstableV6 : wayland::XdgPopupV6
{
public:
    XdgPopupUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id);

    void grab(struct wl_resource* seat, uint32_t serial) override;
    void destroy() override;
};

class XdgToplevelUnstableV6 : public wayland::XdgToplevelV6
{
public:
    XdgToplevelUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id,
                  std::shared_ptr<frontend::Shell> const& shell, XdgSurfaceUnstableV6* self);

    void destroy() override;
    void set_parent(std::experimental::optional<struct wl_resource*> const& parent) override;
    void set_title(std::string const& title) override;
    void set_app_id(std::string const& /*app_id*/) override;
    void show_window_menu(struct wl_resource* seat, uint32_t serial, int32_t x, int32_t y) override;
    void move(struct wl_resource* seat, uint32_t serial) override;
    void resize(struct wl_resource* seat, uint32_t serial, uint32_t edges) override;
    void set_max_size(int32_t width, int32_t height) override;
    void set_min_size(int32_t width, int32_t height) override;
    void set_maximized() override;
    void unset_maximized() override;
    void set_fullscreen(std::experimental::optional<struct wl_resource*> const& output) override;
    void unset_fullscreen() override;
    void set_minimized() override;

private:
    XdgToplevelUnstableV6* get_xdgtoplevel(wl_resource* surface) const;

    std::shared_ptr<frontend::Shell> const shell;
    XdgSurfaceUnstableV6* const self;
};

class XdgPositionerUnstableV6 : public wayland::XdgPositionerV6
{
public:
    XdgPositionerUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id);

    void destroy() override;
    void set_size(int32_t width, int32_t height) override;
    void set_anchor_rect(int32_t x, int32_t y, int32_t width, int32_t height) override;
    void set_anchor(uint32_t anchor) override;
    void set_gravity(uint32_t gravity) override;
    void set_constraint_adjustment(uint32_t constraint_adjustment) override;
    void set_offset(int32_t x, int32_t y) override;

    optional_value<geometry::Size> size;
    optional_value<geometry::Rectangle> aux_rect;
    optional_value<MirPlacementGravity> surface_placement_gravity;
    optional_value<MirPlacementGravity> aux_rect_placement_gravity;
    optional_value<int> aux_rect_placement_offset_x;
    optional_value<int> aux_rect_placement_offset_y;
};

}
}

// XdgShellUnstableV6

mf::XdgShellUnstableV6::XdgShellUnstableV6(struct wl_display* display, std::shared_ptr<mf::Shell> const shell, WlSeat& seat)
    : wayland::XdgShellV6(display, 1),
      shell{shell},
      seat{seat}
{}

void mf::XdgShellUnstableV6::destroy(struct wl_client* client, struct wl_resource* resource)
{
    (void)client, (void)resource;
    // TODO
}

void mf::XdgShellUnstableV6::create_positioner(struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
    new XdgPositionerUnstableV6{client, resource, id};
}

void mf::XdgShellUnstableV6::get_xdg_surface(struct wl_client* client, struct wl_resource* resource, uint32_t id,
                                     struct wl_resource* surface)
{
    new XdgSurfaceUnstableV6{client, resource, id, surface, shell, seat};
}

void mf::XdgShellUnstableV6::pong(struct wl_client* client, struct wl_resource* resource, uint32_t serial)
{
    (void)client, (void)resource, (void)serial;
    // TODO
}

// XdgSurfaceUnstableV6

mf::XdgSurfaceUnstableV6* mf::XdgSurfaceUnstableV6::get_xdgsurface(wl_resource* surface) const
{
    auto* tmp = wl_resource_get_user_data(surface);
    return static_cast<XdgSurfaceUnstableV6*>(static_cast<wayland::XdgSurfaceV6*>(tmp));
}

mf::XdgSurfaceUnstableV6::XdgSurfaceUnstableV6(wl_client* client, wl_resource* parent, uint32_t id, wl_resource* surface,
                               std::shared_ptr<mf::Shell> const& shell, WlSeat& seat)
    : wayland::XdgSurfaceV6(client, parent, id),
      WlAbstractMirWindow{client, surface, resource, shell},
      parent{parent},
      shell{shell},
      sink{std::make_shared<XdgSurfaceUnstableV6EventSink>(&seat, client, surface, resource, destroyed)}
{
    WlAbstractMirWindow::sink = sink;
}

mf::XdgSurfaceUnstableV6::~XdgSurfaceUnstableV6()
{
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(null_wl_mir_window_ptr);
}

void mf::XdgSurfaceUnstableV6::destroy()
{
    wl_resource_destroy(resource);
}

void mf::XdgSurfaceUnstableV6::get_toplevel(uint32_t id)
{
    new XdgToplevelUnstableV6{client, parent, id, shell, this};
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(this);
}

void mf::XdgSurfaceUnstableV6::get_popup(uint32_t id, struct wl_resource* parent, struct wl_resource* positioner)
{
    auto* tmp = wl_resource_get_user_data(positioner);
    auto const* const pos =  static_cast<XdgPositionerUnstableV6*>(static_cast<wayland::XdgPositionerV6*>(tmp));

    auto const session = get_session(client);
    auto& parent_surface = *get_xdgsurface(parent);

    params->type = mir_window_type_freestyle;
    params->parent_id = parent_surface.surface_id;
    if (pos->size.is_set()) params->size = pos->size.value();
    params->aux_rect = pos->aux_rect;
    params->surface_placement_gravity = pos->surface_placement_gravity;
    params->aux_rect_placement_gravity = pos->aux_rect_placement_gravity;
    params->aux_rect_placement_offset_x = pos->aux_rect_placement_offset_x;
    params->aux_rect_placement_offset_y = pos->aux_rect_placement_offset_y;
    params->placement_hints = mir_placement_hints_slide_any;

    new XdgPopupUnstableV6{client, parent, id};
    auto* const mir_surface = WlSurface::from(surface);
    mir_surface->set_role(this);
}

void mf::XdgSurfaceUnstableV6::set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
{
    WlSurface::from(surface)->buffer_offset = geom::Displacement{-x, -y};
    window_size = geom::Size{width, height};
}

void mf::XdgSurfaceUnstableV6::ack_configure(uint32_t serial)
{
    (void)serial;
    // TODO
}


void mf::XdgSurfaceUnstableV6::set_title(std::string const& title)
{
    if (surface_id.as_value())
    {
        spec().name = title;
    }
    else
    {
        params->name = title;
    }
}

void mf::XdgSurfaceUnstableV6::move(struct wl_resource* /*seat*/, uint32_t /*serial*/)
{
    if (surface_id.as_value())
    {
        if (auto session = get_session(client))
        {
            shell->request_operation(session, surface_id, sink->latest_timestamp(), Shell::UserRequest::move);
        }
    }
}

void mf::XdgSurfaceUnstableV6::resize(struct wl_resource* /*seat*/, uint32_t /*serial*/, uint32_t edges)
{
    if (surface_id.as_value())
    {
        if (auto session = get_session(client))
        {
            MirResizeEdge edge = mir_resize_edge_none;

            switch (edges)
            {
            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP:
                edge = mir_resize_edge_north;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM:
                edge = mir_resize_edge_south;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_LEFT:
                edge = mir_resize_edge_west;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_LEFT:
                edge = mir_resize_edge_northwest;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_LEFT:
                edge = mir_resize_edge_southwest;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_RIGHT:
                edge = mir_resize_edge_east;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP_RIGHT:
                edge = mir_resize_edge_northeast;
                break;

            case ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM_RIGHT:
                edge = mir_resize_edge_southeast;
                break;

            default:;
            }

            shell->request_operation(
                session,
                surface_id,
                sink->latest_timestamp(),
                Shell::UserRequest::resize,
                edge);
        }
    }
}

void mf::XdgSurfaceUnstableV6::set_notify_resize(std::function<void(geometry::Size const& new_size)> notify_resize)
{
    sink->notify_resize = notify_resize;
}

void mf::XdgSurfaceUnstableV6::set_parent(optional_value<SurfaceId> parent_id)
{
    if (surface_id.as_value())
    {
        spec().parent_id = parent_id;
    }
    else
    {
        params->parent_id = parent_id;
    }
}

void mf::XdgSurfaceUnstableV6::set_max_size(int32_t width, int32_t height)
{
    if (surface_id.as_value())
    {
        if (width == 0) width = std::numeric_limits<int>::max();
        if (height == 0) height = std::numeric_limits<int>::max();

        auto& mods = spec();
        mods.max_width = geom::Width{width};
        mods.max_height = geom::Height{height};
    }
    else
    {
        if (width == 0)
        {
            if (params->max_width.is_set())
                params->max_width.consume();
        }
        else
            params->max_width = geom::Width{width};

        if (height == 0)
        {
            if (params->max_height.is_set())
                params->max_height.consume();
        }
        else
            params->max_height = geom::Height{height};
    }
}

void mf::XdgSurfaceUnstableV6::set_min_size(int32_t width, int32_t height)
{
    if (surface_id.as_value())
    {
        auto& mods = spec();
        mods.min_width = geom::Width{width};
        mods.min_height = geom::Height{height};
    }
    else
    {
        params->min_width = geom::Width{width};
        params->min_height = geom::Height{height};
    }
}

void mf::XdgSurfaceUnstableV6::set_maximized()
{
    if (surface_id.as_value())
    {
        spec().state = mir_window_state_maximized;
    }
    else
    {
        params->state = mir_window_state_maximized;
    }
}

void mf::XdgSurfaceUnstableV6::unset_maximized()
{
    if (surface_id.as_value())
    {
        spec().state = mir_window_state_restored;
    }
    else
    {
        params->state = mir_window_state_restored;
    }
}

// XdgSurfaceUnstableV6EventSink

mf::XdgSurfaceUnstableV6EventSink::XdgSurfaceUnstableV6EventSink(WlSeat* seat, wl_client* client, wl_resource* target,
                                                 wl_resource* event_sink, std::shared_ptr<bool> const& destroyed)
    : BasicSurfaceEventSink(seat, client, target, event_sink),
      destroyed{destroyed}
{
    auto const serial = wl_display_next_serial(wl_client_get_display(client));
    post_configure(serial);
}

void mf::XdgSurfaceUnstableV6EventSink::send_resize(geometry::Size const& new_size) const
{
    if (window_size != new_size)
    {
        auto const serial = wl_display_next_serial(wl_client_get_display(client));
        notify_resize(new_size);
        post_configure(serial);
    }
}

void mf::XdgSurfaceUnstableV6EventSink::post_configure(int serial) const
{
    seat->spawn(run_unless(destroyed, [event_sink= event_sink, serial]()
        {
            wl_resource_post_event(event_sink, 0, serial);
        }));
}

// XdgPopupUnstableV6

mf::XdgPopupUnstableV6::XdgPopupUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id)
    : wayland::XdgPopupV6(client, parent, id)
{}

void mf::XdgPopupUnstableV6::grab(struct wl_resource* seat, uint32_t serial)
{
    (void)seat, (void)serial;
    // TODO
}

void mf::XdgPopupUnstableV6::destroy()
{
    wl_resource_destroy(resource);
}

// XdgToplevelUnstableV6

mf::XdgToplevelUnstableV6::XdgToplevelUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id,
                                 std::shared_ptr<mf::Shell> const& shell, XdgSurfaceUnstableV6* self)
    : wayland::XdgToplevelV6(client, parent, id),
      shell{shell},
      self{self}
{
    self->set_notify_resize(
        [this](geom::Size const& new_size)
        {
            wl_array states;
            wl_array_init(&states);

            zxdg_toplevel_v6_send_configure(resource, new_size.width.as_int(), new_size.height.as_int(), &states);
        });
}

void mf::XdgToplevelUnstableV6::destroy()
{
    wl_resource_destroy(resource);
}

void mf::XdgToplevelUnstableV6::set_parent(std::experimental::optional<struct wl_resource*> const& parent)
{
    if (parent && parent.value())
    {
        self->set_parent(get_xdgtoplevel(parent.value())->self->surface_id);
    }
    else
    {
        self->set_parent({});
    }

}

void mf::XdgToplevelUnstableV6::set_title(std::string const& title)
{
    self->set_title(title);
}

void mf::XdgToplevelUnstableV6::set_app_id(std::string const& /*app_id*/)
{
    // Logically this sets the session name, but Mir doesn't allow this (currently) and
    // allowing e.g. "session_for_client(client)->name(app_id);" would break the libmirserver ABI
}

void mf::XdgToplevelUnstableV6::show_window_menu(struct wl_resource* seat, uint32_t serial, int32_t x, int32_t y)
{
    (void)seat, (void)serial, (void)x, (void)y;
    // TODO
}

void mf::XdgToplevelUnstableV6::move(struct wl_resource* seat, uint32_t serial)
{
    self->move(seat, serial);
}

void mf::XdgToplevelUnstableV6::resize(struct wl_resource* seat, uint32_t serial, uint32_t edges)
{
    self->resize(seat, serial, edges);
}

void mf::XdgToplevelUnstableV6::set_max_size(int32_t width, int32_t height)
{
    self->set_max_size(width, height);
}

void mf::XdgToplevelUnstableV6::set_min_size(int32_t width, int32_t height)
{
    self->set_min_size(width, height);
}

void mf::XdgToplevelUnstableV6::set_maximized()
{
    self->set_maximized();
}

void mf::XdgToplevelUnstableV6::unset_maximized()
{
    self->unset_maximized();
}

void mf::XdgToplevelUnstableV6::set_fullscreen(std::experimental::optional<struct wl_resource*> const& output)
{
    (void)output;
    // TODO
}

void mf::XdgToplevelUnstableV6::unset_fullscreen()
{
    // TODO
}

void mf::XdgToplevelUnstableV6::set_minimized()
{
    // TODO
}

mf::XdgToplevelUnstableV6* mf::XdgToplevelUnstableV6::get_xdgtoplevel(wl_resource* surface) const
{
    auto* tmp = wl_resource_get_user_data(surface);
    return static_cast<XdgToplevelUnstableV6*>(static_cast<wayland::XdgToplevelV6*>(tmp));
}

// XdgPositionerUnstableV6

mf::XdgPositionerUnstableV6::XdgPositionerUnstableV6(struct wl_client* client, struct wl_resource* parent, uint32_t id)
    : wayland::XdgPositionerV6(client, parent, id)
{}

void mf::XdgPositionerUnstableV6::destroy()
{
    wl_resource_destroy(resource);
}

void mf::XdgPositionerUnstableV6::set_size(int32_t width, int32_t height)
{
    size = geom::Size{width, height};
}

void mf::XdgPositionerUnstableV6::set_anchor_rect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    aux_rect = geom::Rectangle{{x, y}, {width, height}};
}

void mf::XdgPositionerUnstableV6::set_anchor(uint32_t anchor)
{
    MirPlacementGravity placement = mir_placement_gravity_center;

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_TOP)
        placement = MirPlacementGravity(placement | mir_placement_gravity_north);

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_BOTTOM)
        placement = MirPlacementGravity(placement | mir_placement_gravity_south);

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_LEFT)
        placement = MirPlacementGravity(placement | mir_placement_gravity_west);

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_RIGHT)
        placement = MirPlacementGravity(placement | mir_placement_gravity_east);

    surface_placement_gravity = placement;
}

void mf::XdgPositionerUnstableV6::set_gravity(uint32_t gravity)
{
    MirPlacementGravity placement = mir_placement_gravity_center;

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_TOP)
        placement = MirPlacementGravity(placement | mir_placement_gravity_north);

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_BOTTOM)
        placement = MirPlacementGravity(placement | mir_placement_gravity_south);

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_LEFT)
        placement = MirPlacementGravity(placement | mir_placement_gravity_west);

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_RIGHT)
        placement = MirPlacementGravity(placement | mir_placement_gravity_east);

    aux_rect_placement_gravity = placement;
}

void mf::XdgPositionerUnstableV6::set_constraint_adjustment(uint32_t constraint_adjustment)
{
    (void)constraint_adjustment;
    // TODO
}

void mf::XdgPositionerUnstableV6::set_offset(int32_t x, int32_t y)
{
    aux_rect_placement_offset_x = x;
    aux_rect_placement_offset_y = y;
}

