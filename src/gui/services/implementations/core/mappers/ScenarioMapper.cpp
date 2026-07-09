#include "ScenarioMapper.h"

#include "VectorMapper.h"

#include <utility>

namespace gui::coremappers {

core::ScenarioData toCoreScenario(const GuiScenario& scenario)
{
    core::ScenarioData mapped;

    for (const GuiPlane& plane : scenario.planes) {
        if (!plane.visible) {
            continue;
        }

        core::SurfaceData surface;
        surface.id = plane.id;
        surface.absorption = static_cast<double>(plane.absorption);

        for (const GuiTriangle& triangle : plane.triangles) {
            if (!triangle.visible) {
                continue;
            }

            surface.triangles.push_back({
                triangle.id,
                plane.id,
                toCore(triangle.vertices[0]),
                toCore(triangle.vertices[1]),
                toCore(triangle.vertices[2]),
            });
        }

        if (!surface.triangles.empty()) {
            mapped.surfaces.push_back(std::move(surface));
        }
    }

    for (const GuiSource& source : scenario.sources) {
        if (source.visible) {
            mapped.sources.push_back({source.id, toCore(source.position), 1.0});
        }
    }

    for (const GuiReceiver& receiver : scenario.receivers) {
        if (receiver.visible) {
            mapped.receivers.push_back({receiver.id, toCore(receiver.position), 0.5});
        }
    }

    return mapped;
}

}  // namespace gui::coremappers
