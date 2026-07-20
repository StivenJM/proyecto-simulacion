#include "CorePlaneService.h"

namespace gui {

void CorePlaneService::addPlane(GuiScenario& scenario, GuiSelection& selection) { fallback_.addPlane(scenario, selection); }
void CorePlaneService::addRoom(GuiScenario& scenario, GuiSelection& selection) { fallback_.addRoom(scenario, selection); }
void CorePlaneService::addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) { fallback_.addPlaneFromPoints(scenario, selection, points); }
bool CorePlaneService::updatePlanePoint(GuiScenario& scenario, GuiSelection& selection, int planeId, std::size_t pointIndex, Vec3 point) { return fallback_.updatePlanePoint(scenario, selection, planeId, pointIndex, point); }
bool CorePlaneService::updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) { return fallback_.updatePlanePoints(scenario, selection, planeId, points); }
bool CorePlaneService::updatePlaneName(GuiScenario& scenario, int planeId, const std::string& name) { return fallback_.updatePlaneName(scenario, planeId, name); }
bool CorePlaneService::updatePlaneAbsorption(GuiScenario& scenario, int planeId, float absorption) { return fallback_.updatePlaneAbsorption(scenario, planeId, absorption); }
void CorePlaneService::updateAllPlaneAbsorption(GuiScenario& scenario, float absorption) { fallback_.updateAllPlaneAbsorption(scenario, absorption); }
bool CorePlaneService::updatePlaneVisibility(GuiScenario& scenario, int planeId, bool visible) { return fallback_.updatePlaneVisibility(scenario, planeId, visible); }
bool CorePlaneService::updatePlaneColor(GuiScenario& scenario, int planeId, Vec3 color) { return fallback_.updatePlaneColor(scenario, planeId, color); }
void CorePlaneService::selectNext(const GuiScenario& scenario, GuiSelection& selection) { fallback_.selectNext(scenario, selection); }
void CorePlaneService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) { fallback_.moveSelected(scenario, selection, delta); }

}  // namespace gui
