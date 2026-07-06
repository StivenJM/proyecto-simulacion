#pragma once
#include <memory>
#include "AppConfig.h"
#include "ISimulationService.h"

namespace core {

// Crea la implementacion correcta de ISimulationService segun el AppConfig.
// Si serviceMode es Mock retorna MockSimulationService, si es Core retorna CoreSimulationService.
class ServiceFactory {
public:
    static std::unique_ptr<ISimulationService> createSimulationService(
        const AppConfig& appConfig
    );
};

} // namespace core
