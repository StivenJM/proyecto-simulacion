#pragma once

#include "SimulationTypes.h"

#include <vector>

namespace core {

class SurfaceTriangulator {
public:
    static std::vector<SurfaceData> triangulateSurfaces(
        const std::vector<SurfaceData>& surfaces,
        int meshSubdivisions
    );

private:
    static SurfaceData triangulateSurface(
        const SurfaceData& surface,
        int meshSubdivisions,
        int& nextTriangleId
    );
};

} // namespace core
