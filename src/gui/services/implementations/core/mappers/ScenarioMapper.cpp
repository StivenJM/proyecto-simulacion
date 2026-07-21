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

        for (const Vec3& point : plane.outlinePoints) {
            surface.outlinePoints.push_back(toCore(point));
        }

        if (surface.outlinePoints.size() >= 3) {
            mapped.surfaces.push_back(std::move(surface));
        }
    }

    for (const GuiSource& source : scenario.sources) {
        if (source.visible) {
            mapped.sources.push_back({source.id, toCore(source.position), static_cast<double>(source.energy)});
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
