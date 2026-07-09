#pragma once

#include "gui/config/GuiConfig.h"
#include "gui/services/ISimulationService.h"

#include <memory>

namespace gui {

class SimulationServiceFactory {
public:
    static std::unique_ptr<ISimulationService> create(const GuiConfig& config);
};

}  // namespace gui
