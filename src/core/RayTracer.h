#pragma once
#include "SimulationTypes.h"
#include <vector>

namespace core {

class RayTracer {
public:
    struct TraceOutput {
        std::vector<ReflectionRaySegment> rays;
        std::vector<ReceiverEnergySample> receiverEnergy;
        std::vector<TriangleEnergySample> diffuseSeeds;
    };

    // RF-07, RF-08, RF-09, RF-10: Lanza rayos desde la fuente, calcula rebotes especulares,
    // aplica absorcion por superficie y detecta cuando un rayo alcanza un receptor.
    // Respeta el limite temporal definido en config.durationMs.
    static TraceOutput trace(
        const SourceData&                source,
        const std::vector<SurfaceData>&  surfaces,
        const std::vector<ReceiverData>& receivers,
        const SimulationConfig&          config
    );

private:
    // Interseccion rayo-triangulo usando el algoritmo Moller-Trumbore.
    // Retorna la distancia t al punto de impacto, o -1 si no hay interseccion.
    static double intersectRayTriangle(
        const Vec3& origin, const Vec3& direction, const TriangleData& tri
    );

    // Calcula la direccion reflejada de un rayo dado su normal de superficie.
    static Vec3 reflect(const Vec3& direction, const Vec3& normal);

    // Genera direcciones distribuidas en una esfera subdividiendo un icosaedro.
    // El conteo solicitado se ajusta a la forma 2 + 10*n^2.
    static std::vector<Vec3> generateRayDirections(int count);
};

} // namespace core
