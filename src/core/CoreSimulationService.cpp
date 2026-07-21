#include "CoreSimulationService.h"
#include "GeometryCalculator.h"
#include "RayTracer.h"
#include "DiffuseEnergySolver.h"
#include "SurfaceTriangulator.h"
#include "CoreTiming.h"
#include <algorithm>
#include <chrono>

namespace core {

SimulationResult CoreSimulationService::runSimulation(
    const ScenarioData&     scenario,
    const SimulationConfig& config
) {
    const auto simulationStart = std::chrono::steady_clock::now();
    auto elapsedMs = [](const std::chrono::steady_clock::time_point& start) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start
        ).count();
    };

    SimulationResult result;

    if (scenario.sources.empty()) {
        result.success = false;
        result.message = "No hay fuentes definidas en el escenario.";
        coreTiming() << "[CoreTiming] CoreSimulationService::runSimulation durationMs="
                  << elapsedMs(simulationStart)
                  << " status=failed reason=no_sources\n";
        return result;
    }

    const auto triangulationStart = std::chrono::steady_clock::now();
    const std::vector<SurfaceData> triangulatedSurfaces = SurfaceTriangulator::triangulateSurfaces(
        scenario.surfaces,
        config.meshSubdivisions
    );
    std::size_t triangulatedTriangleCount = 0;
    for (const auto& surf : triangulatedSurfaces) triangulatedTriangleCount += surf.triangles.size();
    coreTiming() << "[CoreTiming] SurfaceTriangulator::triangulateSurfaces durationMs="
              << elapsedMs(triangulationStart)
              << " inputSurfaceCount=" << scenario.surfaces.size()
              << " outputSurfaceCount=" << triangulatedSurfaces.size()
              << " triangleCount=" << triangulatedTriangleCount
              << " meshSubdivisions=" << config.meshSubdivisions << "\n";

    const auto flattenStart = std::chrono::steady_clock::now();
    std::vector<TriangleData> allTriangles;
    for (const auto& surf : triangulatedSurfaces)
        for (const auto& tri : surf.triangles)
            allTriangles.push_back(tri);
    coreTiming() << "[CoreTiming] CoreSimulationService::collectTriangles durationMs="
              << elapsedMs(flattenStart)
              << " triangleCount=" << allTriangles.size() << "\n";

    if (allTriangles.empty()) {
        result.success = false;
        result.message = "No hay geometria definida en el escenario.";
        coreTiming() << "[CoreTiming] CoreSimulationService::runSimulation durationMs="
                  << elapsedMs(simulationStart)
                  << " status=failed reason=no_geometry\n";
        return result;
    }

    const bool useHierarchicalDiffusion =
        config.useHierarchicalDiffusion && config.diffusionSolverMode == DiffusionSolverMode::Hierarchical;
    if (useHierarchicalDiffusion) {
        result.hierarchicalDiffusion = GeometryCalculator::buildHierarchicalDiffusionData(
            allTriangles,
            triangulatedSurfaces,
            config
        );
    }

    if (!useHierarchicalDiffusion || config.exportDenseDiffusionMatrices) {
        const auto denseMatrixStart = std::chrono::steady_clock::now();
        result.diffusion = GeometryCalculator::buildDiffusionMatrix(allTriangles, config.soundSpeed);
        if (useHierarchicalDiffusion) {
            result.hierarchicalDiffusion.metrics.denseMatrixExportDurationMs = elapsedMs(denseMatrixStart);
            result.hierarchicalDiffusion.metrics.denseMatrixExported = true;
        }
    }
    result.diffusionTriangles = allTriangles;

    const int durationMs = std::max(0, std::min(config.durationMs, 1000));
    result.diffuseEnergyByTriangleTime.assign(
        allTriangles.size(),
        std::vector<double>(static_cast<std::size_t>(durationMs + 1), 0.0)
    );

    double totalInitialEnergy = 0.0;
    for (const auto& src : scenario.sources) totalInitialEnergy += src.energy;

    for (const auto& src : scenario.sources) {
        // RF-07 a RF-10: ray tracing especular
        auto traceOut = RayTracer::trace(src, triangulatedSurfaces, scenario.receivers, config);
        const auto mergeTraceStart = std::chrono::steady_clock::now();
        for (auto& seg    : traceOut.rays)          result.reflectionRays.push_back(std::move(seg));
        for (auto& sample : traceOut.receiverEnergy) result.receiverEnergy.push_back(std::move(sample));
        coreTiming() << "[CoreTiming] CoreSimulationService::mergeTraceOutput durationMs="
                  << elapsedMs(mergeTraceStart)
                  << " sourceId=" << src.id
                  << " raySegmentCount=" << result.reflectionRays.size()
                  << " receiverSampleCount=" << result.receiverEnergy.size() << "\n";

        // RF-07, RF-08: energia difusa sembrada por impactos reales de rayos.
        auto diffuseResult = useHierarchicalDiffusion
            ? DiffuseEnergySolver::solveHierarchical(
                result.hierarchicalDiffusion, allTriangles, triangulatedSurfaces, traceOut.diffuseSeeds, config
              )
            : DiffuseEnergySolver::solveDetailed(
                result.diffusion, allTriangles, triangulatedSurfaces, traceOut.diffuseSeeds, config
              );
        const auto mergeDiffuseStart = std::chrono::steady_clock::now();
        for (auto& sample : diffuseResult.samples) result.triangleEnergy.push_back(std::move(sample));
        for (std::size_t triangleIndex = 0; triangleIndex < diffuseResult.energyByTriangleTime.size(); ++triangleIndex) {
            for (std::size_t timeIndex = 0; timeIndex < diffuseResult.energyByTriangleTime[triangleIndex].size(); ++timeIndex) {
                result.diffuseEnergyByTriangleTime[triangleIndex][timeIndex] += diffuseResult.energyByTriangleTime[triangleIndex][timeIndex];
            }
        }
        coreTiming() << "[CoreTiming] CoreSimulationService::mergeDiffuseOutput durationMs="
                  << elapsedMs(mergeDiffuseStart)
                  << " sourceId=" << src.id
                  << " triangleSampleCount=" << result.triangleEnergy.size() << "\n";
    }

    result.totalReceiverEnergy = 0.0;
    for (const auto& s : result.receiverEnergy) result.totalReceiverEnergy += s.energy;
    result.lostEnergy = std::max(0.0, totalInitialEnergy - result.totalReceiverEnergy);

    result.success = true;
    result.message = "Simulacion completada.";
    coreTiming() << "[CoreTiming] CoreSimulationService::runSimulation durationMs="
              << elapsedMs(simulationStart)
              << " status=success"
              << " sourceCount=" << scenario.sources.size()
              << " receiverCount=" << scenario.receivers.size()
              << " triangleCount=" << allTriangles.size()
              << " reflectionSegmentCount=" << result.reflectionRays.size()
              << " receiverSampleCount=" << result.receiverEnergy.size()
              << " diffuseSampleCount=" << result.triangleEnergy.size() << "\n";
    return result;
}

} // namespace core
