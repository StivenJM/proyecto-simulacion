#include "SimulationServiceFactory.h"

#include "gui/services/implementations/core/CoreSimulationService.h"
#include "gui/services/implementations/mock/MockSimulationService.h"

namespace gui {

std::unique_ptr<ISimulationService> SimulationServiceFactory::create(const GuiConfig& config)
{
    if (config.serviceProvider == GuiServiceProvider::Core) {
        return std::make_unique<CoreSimulationService>();
    }

    return std::make_unique<MockSimulationService>();
}

}  // namespace gui
