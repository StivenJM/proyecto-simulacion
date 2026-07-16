#pragma once
#include "SimulationTypes.h"

namespace services {

class ISimulationService {
public:
    virtual ~ISimulationService() = default;

    virtual SimulationResult runSimulation(
        const ScenarioData&    scenario,
        const SimulationConfig& config
    ) = 0;
};

} // namespace services
