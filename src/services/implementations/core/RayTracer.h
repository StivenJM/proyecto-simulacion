#pragma once
#include "services/SimulationTypes.h"
#include <vector>

namespace services {
namespace core {

class RayTracer {
public:
    struct TraceOutput {
        std::vector<ReflectionRaySegment> rays;
        std::vector<ReceiverEnergySample> receiverEnergy;
    };

    static TraceOutput trace(
        const SourceData&              source,
        const std::vector<SurfaceData>& surfaces,
        const std::vector<ReceiverData>& receivers,
        const SimulationConfig&         config
    );

private:
    static double intersectRayTriangle(
        const Vec3& origin, const Vec3& direction, const TriangleData& tri
    );
    static Vec3 reflect(const Vec3& direction, const Vec3& normal);
    static std::vector<Vec3> generateRayDirections(int count);
};

} // namespace core
} // namespace services
