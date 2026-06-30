#pragma once
#include "services/SimulationTypes.h"
#include <vector>

namespace services {
namespace core {

class DiffuseEnergySolver {
public:
    static std::vector<TriangleEnergySample> solve(
        const DiffusionMatrixData&       diffusion,
        const std::vector<TriangleData>& triangles,
        const std::vector<SurfaceData>&  surfaces,
        double                           initialEnergy,
        const SimulationConfig&          config
    );
};

} // namespace core
} // namespace services
