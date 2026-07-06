#pragma once
#include "SimulationTypes.h"
#include <vector>

namespace core {

class DiffuseEnergySolver {
public:
    // RF-07, RF-08, RF-10: Distribuye energia difusa entre los triangulos visibles a lo largo
    // del tiempo, aplicando absorcion por superficie en cada paso y respetando el
    // limite temporal definido en config.durationMs.
    static std::vector<TriangleEnergySample> solve(
        const DiffusionMatrixData&       diffusion,
        const std::vector<TriangleData>& triangles,
        const std::vector<SurfaceData>&  surfaces,
        double                           initialEnergy,
        const SimulationConfig&          config
    );
};

} // namespace core
