#include "DiffuseEnergySolver.h"
#include <algorithm>
#include <numeric>

namespace services {
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

    // Mapa triangleId -> absorción
    auto getAbsorption = [&](int triangleId) -> double {
        for (const auto& surf : surfaces) {
            for (const auto& tri : surf.triangles) {
                if (tri.id == triangleId) return surf.absorption;
            }
        }
        return 0.1;
    };

    // Energía inicial distribuida equitativamente entre todos los triángulos
    std::vector<double> energy(n, initialEnergy / n);

    const int timeStepMs = 10;

    for (int timeMs = 0; timeMs <= config.durationMs; timeMs += timeStepMs) {
        std::vector<double> next(n, 0.0);

        double totalEnergy = std::accumulate(energy.begin(), energy.end(), 0.0);
        if (totalEnergy < 1e-9) break;

        for (int i = 0; i < n; i++) {
            if (energy[i] < 1e-10) continue;

            // Registrar muestra de este triángulo en este instante
            samples.push_back({triangles[i].id, timeMs, energy[i]});

            // Aplicar absorción de la superficie
            double absorption    = getAbsorption(triangles[i].id);
            double afterAbsorb   = energy[i] * (1.0 - absorption);

            // Distribuir energía restante hacia triángulos visibles
            for (int j = 0; j < n; j++) {
                if (i == j) continue;
                if (!diffusion.visibility[i][j]) continue;

                // Solo distribuir si el sonido puede llegar dentro del tiempo restante
                int arrivalMs = timeMs + diffusion.timesMs[i][j];
                if (arrivalMs > config.durationMs) continue;

                next[j] += afterAbsorb * diffusion.percentages[i][j];
            }
        }

        energy = next;
    }

    return samples;
}

} // namespace core
} // namespace services
