#pragma once
#include <memory>
#include "config/AppConfig.h"
#include "services/ISimulationService.h"

namespace services {

class ServiceFactory {
public:
    static std::unique_ptr<ISimulationService> createSimulationService(
        const config::AppConfig& appConfig
    );
};

} // namespace services
