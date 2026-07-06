#pragma once
#include "ISimulationService.h"

namespace core {

// Implementacion falsa para desarrollo de GUI sin necesitar el Core completo.
// Retorna datos coherentes pero inventados.
class MockSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData&     scenario,
        const SimulationConfig& config
    ) override;
};

} // namespace core
