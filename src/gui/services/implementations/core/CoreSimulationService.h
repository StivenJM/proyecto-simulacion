#pragma once

#include "core/ISimulationService.h"
#include "gui/services/ISimulationService.h"

#include <memory>

namespace gui {

class CoreSimulationService final : public ISimulationService {
public:
    CoreSimulationService();
    explicit CoreSimulationService(std::unique_ptr<core::ISimulationService> coreService);

    SimulationResultDto start(const GuiScenario& scenario) override;

private:
    std::unique_ptr<core::ISimulationService> coreService_;
};

}  // namespace gui
