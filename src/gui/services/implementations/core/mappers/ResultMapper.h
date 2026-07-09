#pragma once

#include "core/SimulationTypes.h"
#include "gui/entities/GuiScenario.h"
#include "gui/services/dtos/SimulationDtos.h"

namespace gui::coremappers {

SimulationResultDto toGuiResult(const core::SimulationResult& result, const GuiScenario& scenario);

}  // namespace gui::coremappers
