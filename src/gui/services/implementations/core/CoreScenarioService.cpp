#include "CoreScenarioService.h"

namespace gui {

GuiScenario CoreScenarioService::createInitialScenario(GuiSelection& selection)
{
    return fallback_.createInitialScenario(selection);
}

}  // namespace gui
