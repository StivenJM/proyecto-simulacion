#include "DiffuseEnergySolver.h"
#include <numeric>

namespace core {

std::vector<TriangleEnergySample> DiffuseEnergySolver::solve(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    double                           initialEnergy,
    const SimulationConfig&          config
) {
    std::vector<TriangleEnergySample> samples;

    int n = static_cast<int>(triangles.size());
    if (n == 0) return samples;

    auto getAbsorption = [&](int triangleId) -> double {
        for (const auto& surf : surfaces)
            for (const auto& tri : surf.triangles)
                if (tri.id == triangleId) return surf.absorption;
        return 0.1;
    };

    std::vector<double> energy(n, initialEnergy / n);

    const int timeStepMs = 10;

    for (int timeMs = 0; timeMs <= config.durationMs; timeMs += timeStepMs) {
        double total = std::accumulate(energy.begin(), energy.end(), 0.0);
        if (total < 1e-9) break;

        std::vector<double> next(n, 0.0);

        for (int i = 0; i < n; i++) {
            if (energy[i] < 1e-10) continue;

            samples.push_back({triangles[i].id, timeMs, energy[i]});

            // RF-08: aplicar absorcion de la superficie
            double afterAbsorb = energy[i] * (1.0 - getAbsorption(triangles[i].id));

            // RF-07: distribuir hacia triangulos visibles respetando tiempos (RF-05) y porcentajes (RF-06)
            for (int j = 0; j < n; j++) {
                if (i == j || !diffusion.visibility[i][j]) continue;
                if (timeMs + diffusion.timesMs[i][j] > config.durationMs) continue;
                next[j] += afterAbsorb * diffusion.percentages[i][j];
            }
        }

        energy = next;
    }

    return samples;
}

} // namespace core
