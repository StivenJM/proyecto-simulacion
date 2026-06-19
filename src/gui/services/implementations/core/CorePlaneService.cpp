#include "CorePlaneService.h"

namespace gui {

void CorePlaneService::addPlane(GuiScenario& scenario, GuiSelection& selection) { fallback_.addPlane(scenario, selection); }
void CorePlaneService::addRoom(GuiScenario& scenario, GuiSelection& selection) { fallback_.addRoom(scenario, selection); }
void CorePlaneService::addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) { fallback_.addPlaneFromPoints(scenario, selection, points); }
bool CorePlaneService::updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) { return fallback_.updatePlanePoints(scenario, selection, planeId, points); }
void CorePlaneService::selectNext(const GuiScenario& scenario, GuiSelection& selection) { fallback_.selectNext(scenario, selection); }
void CorePlaneService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) { fallback_.moveSelected(scenario, selection, delta); }

}  // namespace gui
