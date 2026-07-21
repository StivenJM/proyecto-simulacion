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
        const std::vector<TriangleEnergySample>& initialSeeds,
        const SimulationConfig&          config
    );

    static DiffuseEnergyResult solveDetailed(
        const DiffusionMatrixData&       diffusion,
        const std::vector<TriangleData>& triangles,
        const std::vector<SurfaceData>&  surfaces,
        const std::vector<TriangleEnergySample>& initialSeeds,
        const SimulationConfig&          config
    );

    static DiffuseEnergyResult solveHierarchical(
        HierarchicalDiffusionData&       hierarchy,
        const std::vector<TriangleData>& triangles,
        const std::vector<SurfaceData>&  surfaces,
        const std::vector<TriangleEnergySample>& initialSeeds,
        const SimulationConfig&          config
    );
};

} // namespace core
