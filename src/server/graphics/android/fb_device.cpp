/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/compositor/buffer.h"

#include "fb_device.h"
#include "android_buffer.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace mga=mir::graphics::android;
namespace geom=mir::geometry;
 
mga::FBDevice::FBDevice(std::shared_ptr<framebuffer_device_t> const& fbdev)
    : fb_device(fbdev)
{
}

void mga::FBDevice::set_next_frontbuffer(std::shared_ptr<AndroidBuffer> const& buffer)
{
    auto handle = buffer->native_buffer_handle();
    if (fb_device->post(fb_device.get(), handle->handle) != 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("error posting with fb device"));
    }
}

geom::Size mga::FBDevice::display_size() const
{
    return geom::Size{geom::Width{fb_device->width},
                      geom::Height{fb_device->height}};
} 

geom::PixelFormat mga::FBDevice::display_format() const
{
    if (fb_device->format == HAL_PIXEL_FORMAT_RGBA_8888)
    {
        return geom::PixelFormat::abgr_8888;
    }
    if (fb_device->format == HAL_PIXEL_FORMAT_BGRA_8888)
    {
        return geom::PixelFormat::argb_8888;
    }
    return geom::PixelFormat::invalid; 
}

unsigned int mga::FBDevice::number_of_framebuffers_available() const
{
    auto fb_num = static_cast<unsigned int>(fb_device->numFramebuffers);
    return std::max(2u, fb_num);
}
