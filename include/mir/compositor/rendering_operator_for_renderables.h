/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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
#ifndef MIR_COMPOSITOR_RENDERING_OPERATOR_FOR_RENDERABLES_H_
#define MIR_COMPOSITOR_RENDERING_OPERATOR_FOR_RENDERABLES_H_

#include "mir/graphics/renderer.h"
#include "mir/graphics/renderable.h"
#include "mir/compositor/render_view.h"
#include "mir/compositor/graphic_region.h"
#include <vector>

namespace mir
{
namespace compositor
{
class RenderingOperatorForRenderables : public OperatorForRenderables
{
public:
    explicit RenderingOperatorForRenderables(graphics::Renderer& renderer);

    void operator()(graphics::Renderable& renderable);

private:
    graphics::Renderer& renderer;

    std::vector<std::shared_ptr<compositor::GraphicBufferCompositorResource>> compositor_resources;
    std::vector<std::shared_ptr<compositor::GraphicRegion>> texture_resources;
};

}
}
#endif /* MIR_COMPOSITOR_RENDERING_OPERATOR_FOR_RENDERABLES_H_ */
