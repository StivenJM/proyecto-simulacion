#pragma once

#include "gui/services/ISimulationService.h"

namespace gui {

class MockSimulationService final : public ISimulationService {
public:
    SimulationResultDto start(const GuiScenario& scenario) override;
};

}  // namespace gui
