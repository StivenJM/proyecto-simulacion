#include "MockScenarioService.h"

namespace gui {

GuiScenario MockScenarioService::createInitialScenario(GuiSelection& selection)
{
    GuiScenario scenario;
    planeService_.addRoom(scenario, selection);
    return scenario;
}

}  // namespace gui
