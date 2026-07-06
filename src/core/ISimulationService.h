#pragma once
#include "SimulationTypes.h"

namespace core {

// Interfaz que la GUI consume para ejecutar simulaciones.
// La implementacion puede ser Mock (datos falsos) o Core (calculo real).
class ISimulationService {
public:
    virtual ~ISimulationService() = default;

    // Ejecuta la simulacion sobre el escenario dado y retorna los resultados. (RF-01 a RF-11)
    virtual SimulationResult runSimulation(
        const ScenarioData&     scenario,
        const SimulationConfig& config
    ) = 0;
};

} // namespace core
