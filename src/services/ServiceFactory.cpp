#include "ServiceFactory.h"
#include "config/ServiceMode.h"
#include "services/implementations/mock/MockSimulationService.h"
#include "services/implementations/core/CoreSimulationService.h"

namespace services {

std::unique_ptr<ISimulationService> ServiceFactory::createSimulationService(
    const config::AppConfig& appConfig
) {
    if (appConfig.serviceMode == config::ServiceMode::Mock) {
        return std::make_unique<mock::MockSimulationService>();
    }
    return std::make_unique<core::CoreSimulationService>();
}

} // namespace services
