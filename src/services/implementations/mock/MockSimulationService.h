#pragma once
#include "services/ISimulationService.h"

namespace services {
namespace mock {

class MockSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData&     scenario,
        const SimulationConfig& config
    ) override;
};

} // namespace mock
} // namespace services
