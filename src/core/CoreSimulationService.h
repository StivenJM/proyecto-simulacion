#pragma once
#include "ISimulationService.h"

namespace core {

// Implementacion real de la simulacion acustica.
// Orquesta GeometryCalculator, RayTracer y DiffuseEnergySolver. (RF-01 a RF-11)
class CoreSimulationService : public ISimulationService {
public:
    SimulationResult runSimulation(
        const ScenarioData&     scenario,
        const SimulationConfig& config
    ) override;
};

} // namespace core
