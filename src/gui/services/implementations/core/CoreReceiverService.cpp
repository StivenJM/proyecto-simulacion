#include "CoreReceiverService.h"

namespace gui {

void CoreReceiverService::addReceiver(GuiScenario& scenario, GuiSelection& selection) { fallback_.addReceiver(scenario, selection); }
bool CoreReceiverService::updateReceiverPosition(GuiScenario& scenario, int receiverId, Vec3 position) { return fallback_.updateReceiverPosition(scenario, receiverId, position); }
bool CoreReceiverService::updateReceiverName(GuiScenario& scenario, int receiverId, const std::string& name) { return fallback_.updateReceiverName(scenario, receiverId, name); }
bool CoreReceiverService::updateReceiverVisibility(GuiScenario& scenario, int receiverId, bool visible) { return fallback_.updateReceiverVisibility(scenario, receiverId, visible); }
void CoreReceiverService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) { fallback_.moveSelected(scenario, selection, delta); }

}  // namespace gui
