#include "CoreSimulationService.h"
#include "GeometryCalculator.h"
#include "RayTracer.h"
#include "DiffuseEnergySolver.h"
#include <algorithm>

namespace services {
namespace core {

SimulationResult CoreSimulationService::runSimulation(
    const ScenarioData&     scenario,
    const SimulationConfig& config
) {
    SimulationResult result;

    if (scenario.sources.empty()) {
        result.success = false;
        result.message = "No hay fuentes definidas en el escenario.";
        return result;
    }

    // Aplanar todos los triángulos de todas las superficies
    std::vector<TriangleData> allTriangles;
    for (const auto& surf : scenario.surfaces) {
        for (const auto& tri : surf.triangles) {
            allTriangles.push_back(tri);
        }
    }

    if (allTriangles.empty()) {
        result.success = false;
        result.message = "No hay geometría definida en el escenario.";
        return result;
    }

    // Construir matriz de difusión (distancias, tiempos, visibilidad, porcentajes)
    result.diffusion = GeometryCalculator::buildDiffusionMatrix(
        allTriangles, config.soundSpeed
    );

    double totalInitialEnergy = 0.0;
    for (const auto& src : scenario.sources) {
        totalInitialEnergy += src.energy;
    }

    // Procesar cada fuente
    for (const auto& src : scenario.sources) {
        // Ray tracing especular
        RayTracer::TraceOutput traceOut = RayTracer::trace(
            src, scenario.surfaces, scenario.receivers, config
        );

        for (auto& seg    : traceOut.rays)          result.reflectionRays.push_back(std::move(seg));
        for (auto& sample : traceOut.receiverEnergy) result.receiverEnergy.push_back(std::move(sample));

        // Difusión de energía entre triángulos
        double diffuseEnergy = src.energy * config.diffusionCoefficient;
        auto triSamples = DiffuseEnergySolver::solve(
            result.diffusion, allTriangles, scenario.surfaces, diffuseEnergy, config
        );
        for (auto& sample : triSamples) result.triangleEnergy.push_back(std::move(sample));
    }

    // Calcular totales
    result.totalReceiverEnergy = 0.0;
    for (const auto& s : result.receiverEnergy) {
        result.totalReceiverEnergy += s.energy;
    }
    result.lostEnergy = std::max(0.0, totalInitialEnergy - result.totalReceiverEnergy);

    result.success = true;
    result.message = "Simulación completada.";
    return result;
}

} // namespace core
} // namespace services
