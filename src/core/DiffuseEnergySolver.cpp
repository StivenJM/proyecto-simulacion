#include "DiffuseEnergySolver.h"
#include <numeric>
#include <algorithm>
#include <unordered_map>

namespace core {

std::vector<TriangleEnergySample> DiffuseEnergySolver::solve(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    std::vector<TriangleEnergySample> samples;

    int n = static_cast<int>(triangles.size());
    if (n == 0) return samples;

    auto getAbsorption = [&](int triangleId) -> double {
        for (const auto& surf : surfaces)
            for (const auto& tri : surf.triangles)
                if (tri.id == triangleId) return surf.absorption;
        return 0.2;
    };

    const int durationMs = std::max(0, std::min(config.durationMs, 1000));
    std::vector<std::vector<double>> arrivals(
        static_cast<std::size_t>(durationMs + 1),
        std::vector<double>(static_cast<std::size_t>(n), 0.0)
    );

    std::unordered_map<int, int> triangleIndexById;
    for (int i = 0; i < n; ++i) triangleIndexById[triangles[i].id] = i;

    for (const auto& seed : initialSeeds) {
        auto it = triangleIndexById.find(seed.triangleId);
        if (it == triangleIndexById.end()) continue;
        if (seed.timeMs < 0 || seed.timeMs > durationMs) continue;
        if (seed.energy <= 0.0) continue;
        arrivals[seed.timeMs][it->second] += seed.energy;
    }

    for (int timeMs = 0; timeMs <= durationMs; ++timeMs) {
        std::vector<double>& energy = arrivals[timeMs];
        double total = std::accumulate(energy.begin(), energy.end(), 0.0);
        if (total < 1e-9) continue;

        for (int i = 0; i < n; i++) {
            if (energy[i] < 1e-10) continue;

            samples.push_back({triangles[i].id, triangles[i].surfaceId, timeMs, energy[i]});

            // RF-08: aplicar absorcion de la superficie
            double afterAbsorb = energy[i] * (1.0 - getAbsorption(triangles[i].id));

            // RF-07: distribuir hacia triangulos visibles respetando tiempos (RF-05) y porcentajes (RF-06)
            for (int j = 0; j < n; j++) {
                if (i == j || !diffusion.visibility[i][j]) continue;
                const int arrivalMs = timeMs + diffusion.timesMs[i][j];
                if (arrivalMs > durationMs) continue;
                arrivals[arrivalMs][j] += afterAbsorb * diffusion.percentages[i][j];
            }
        }
    }

    return samples;
}

} // namespace core
