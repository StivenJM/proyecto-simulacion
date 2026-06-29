#include "CoreSourceService.h"

namespace gui {

void CoreSourceService::addSource(GuiScenario& scenario, GuiSelection& selection) { fallback_.addSource(scenario, selection); }
bool CoreSourceService::updateSourcePosition(GuiScenario& scenario, int sourceId, Vec3 position) { return fallback_.updateSourcePosition(scenario, sourceId, position); }
bool CoreSourceService::updateSourceName(GuiScenario& scenario, int sourceId, const std::string& name) { return fallback_.updateSourceName(scenario, sourceId, name); }
bool CoreSourceService::updateSourceVisibility(GuiScenario& scenario, int sourceId, bool visible) { return fallback_.updateSourceVisibility(scenario, sourceId, visible); }
void CoreSourceService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) { fallback_.moveSelected(scenario, selection, delta); }

}  // namespace gui
