#pragma once

#include "core/SimulationTypes.h"
#include "gui/entities/GuiScenario.h"

namespace gui::coremappers {

core::ScenarioData toCoreScenario(const GuiScenario& scenario);

}  // namespace gui::coremappers
