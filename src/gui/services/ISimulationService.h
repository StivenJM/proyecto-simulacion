#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/services/dtos/SimulationDtos.h"

namespace gui {

class ISimulationService {
public:
    virtual ~ISimulationService() = default;

    virtual SimulationResultDto start(const GuiScenario& scenario) = 0;
};

}  // namespace gui
