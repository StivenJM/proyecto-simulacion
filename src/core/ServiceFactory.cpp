#include "ServiceFactory.h"
#include "ServiceMode.h"
#include "MockSimulationService.h"
#include "CoreSimulationService.h"

namespace core {

std::unique_ptr<ISimulationService> ServiceFactory::createSimulationService(
    const AppConfig& appConfig
) {
    if (appConfig.serviceMode == ServiceMode::Mock)
        return std::make_unique<MockSimulationService>();
    return std::make_unique<CoreSimulationService>();
}

} // namespace core
