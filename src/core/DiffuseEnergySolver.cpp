#include "DiffuseEnergySolver.h"
#include "CoreTiming.h"
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <unordered_map>

namespace core {

std::vector<TriangleEnergySample> DiffuseEnergySolver::solve(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    return solveDetailed(diffusion, triangles, surfaces, initialSeeds, config).samples;
}

DiffuseEnergyResult DiffuseEnergySolver::solveDetailed(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    const auto start = std::chrono::steady_clock::now();
    DiffuseEnergyResult result;
    std::vector<TriangleEnergySample> samples;

    int n = static_cast<int>(triangles.size());
    if (n == 0) {
        coreTiming() << "[CoreTiming] DiffuseEnergySolver::solveDetailed durationMs=0"
                  << " triangleCount=0"
                  << " simulationDurationMs=" << std::max(0, std::min(config.durationMs, 1000))
                  << " seedCount=" << initialSeeds.size()
                  << " acceptedSeedCount=0"
                  << " sampleCount=0"
                  << " activeCellCount=0"
                  << " propagationCount=0\n";
        return result;
    }

    const int durationMs = std::max(0, std::min(config.durationMs, 1000));
    std::size_t acceptedSeedCount = 0;
    long long activeCellCount = 0;
    long long propagationCount = 0;
    std::vector<std::vector<double>> arrivals(
        static_cast<std::size_t>(durationMs + 1),
        std::vector<double>(static_cast<std::size_t>(n), 0.0)
    );
    result.energyByTriangleTime.assign(
        static_cast<std::size_t>(n),
        std::vector<double>(static_cast<std::size_t>(durationMs + 1), 0.0)
    );

    std::unordered_map<int, int> triangleIndexById;
    std::vector<double> absorptionByTriangle(static_cast<std::size_t>(n), 0.2);
    for (int i = 0; i < n; ++i) triangleIndexById[triangles[i].id] = i;

    for (const auto& surf : surfaces) {
        for (const auto& tri : surf.triangles) {
            auto it = triangleIndexById.find(tri.id);
            if (it != triangleIndexById.end()) {
                absorptionByTriangle[static_cast<std::size_t>(it->second)] = surf.absorption;
            }
        }
    }

    for (const auto& seed : initialSeeds) {
        auto it = triangleIndexById.find(seed.triangleId);
        if (it == triangleIndexById.end()) continue;
        if (seed.timeMs < 0 || seed.timeMs > durationMs) continue;
        if (seed.energy <= 0.0) continue;
        arrivals[seed.timeMs][it->second] += seed.energy;
        ++acceptedSeedCount;
    }

    for (int timeMs = 0; timeMs <= durationMs; ++timeMs) {
        std::vector<double>& energy = arrivals[timeMs];
        double total = std::accumulate(energy.begin(), energy.end(), 0.0);
        if (total < 1e-9) continue;

        for (int i = 0; i < n; i++) {
            if (energy[i] < 1e-10) continue;
            ++activeCellCount;

            samples.push_back({triangles[i].id, triangles[i].surfaceId, timeMs, energy[i]});
            result.energyByTriangleTime[static_cast<std::size_t>(i)][static_cast<std::size_t>(timeMs)] += energy[i];

            // RF-08: aplicar absorcion de la superficie
            double afterAbsorb = energy[i] * (1.0 - absorptionByTriangle[static_cast<std::size_t>(i)]);
            if (std::abs(afterAbsorb) < 1e-10) continue;

            // RF-07: distribuir hacia triangulos visibles respetando tiempos (RF-05) y porcentajes (RF-06)
            for (int j = 0; j < n; j++) {
                if (i == j || !diffusion.visibility[i][j]) continue;
                const int arrivalMs = timeMs + diffusion.timesMs[i][j];
                if (arrivalMs > durationMs) continue;
                arrivals[arrivalMs][j] += afterAbsorb * diffusion.percentages[i][j];
                ++propagationCount;
            }
        }
    }

    result.samples = std::move(samples);
    const auto solveDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    coreTiming() << "[CoreTiming] DiffuseEnergySolver::solveDetailed durationMs="
              << solveDurationMs
              << " triangleCount=" << n
              << " simulationDurationMs=" << durationMs
              << " seedCount=" << initialSeeds.size()
              << " acceptedSeedCount=" << acceptedSeedCount
              << " sampleCount=" << result.samples.size()
              << " activeCellCount=" << activeCellCount
              << " propagationCount=" << propagationCount << "\n";
    return result;
}

} // namespace core
