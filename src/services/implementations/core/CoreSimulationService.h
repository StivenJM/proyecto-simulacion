#pragma once
#include "services/ISimulationService.h"

namespace services {
namespace core {

class CoreSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData&     scenario,
        const SimulationConfig& config
    ) override;
};

} // namespace core
} // namespace services
