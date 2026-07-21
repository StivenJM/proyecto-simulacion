#include "CoreSimulationService.h"
#include "GeometryCalculator.h"
#include "RayTracer.h"
#include "DiffuseEnergySolver.h"
#include "SurfaceTriangulator.h"
#include <algorithm>

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

    const std::vector<SurfaceData> triangulatedSurfaces = SurfaceTriangulator::triangulateSurfaces(
        scenario.surfaces,
        config.meshSubdivisions
    );

    std::vector<TriangleData> allTriangles;
    for (const auto& surf : triangulatedSurfaces)
        for (const auto& tri : surf.triangles)
            allTriangles.push_back(tri);

    if (allTriangles.empty()) {
        result.success = false;
        result.message = "No hay geometria definida en el escenario.";
        return result;
    }

    // RF-03 a RF-06: construir matriz de difusion
    result.diffusion = GeometryCalculator::buildDiffusionMatrix(allTriangles, config.soundSpeed);

    double totalInitialEnergy = 0.0;
    for (const auto& src : scenario.sources) totalInitialEnergy += src.energy;

    for (const auto& src : scenario.sources) {
        // RF-07 a RF-10: ray tracing especular
        auto traceOut = RayTracer::trace(src, triangulatedSurfaces, scenario.receivers, config);
        for (auto& seg    : traceOut.rays)          result.reflectionRays.push_back(std::move(seg));
        for (auto& sample : traceOut.receiverEnergy) result.receiverEnergy.push_back(std::move(sample));

        // RF-07, RF-08: energia difusa sembrada por impactos reales de rayos.
        auto triSamples = DiffuseEnergySolver::solve(
            result.diffusion, allTriangles, triangulatedSurfaces, traceOut.diffuseSeeds, config
        );
        for (auto& sample : triSamples) result.triangleEnergy.push_back(std::move(sample));
    }

    result.totalReceiverEnergy = 0.0;
    for (const auto& s : result.receiverEnergy) result.totalReceiverEnergy += s.energy;
    result.lostEnergy = std::max(0.0, totalInitialEnergy - result.totalReceiverEnergy);

    result.success = true;
    result.message = "Simulacion completada.";
    return result;
}

} // namespace core
