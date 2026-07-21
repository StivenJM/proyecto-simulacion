#pragma once
#include <string>
#include <vector>

namespace core {

// Punto o vector en el espacio 3D.
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// Un triángulo que forma parte de una superficie de la sala.
struct TriangleData {
    int  id        = 0;
    int  surfaceId = 0;
    Vec3 a;
    Vec3 b;
    Vec3 c;
};

// Una superficie de la sala, compuesta por uno o más triángulos.
struct SurfaceData {
    int                       id         = 0;
    double                    absorption = 0.2;
    std::vector<Vec3>         outlinePoints;
    std::vector<TriangleData> triangles;
};

// Fuente de energía sonora dentro del escenario.
struct SourceData {
    int    id     = 0;
    Vec3   position;
    double energy = 1.0;
};

// Receptor que mide la energía sonora recibida.
struct ReceiverData {
    int    id     = 0;
    Vec3   position;
    double radius = 0.5;
};

// Escenario completo: superficies, fuentes y receptores. (RF-01, RF-13, RF-14, RF-15)
struct ScenarioData {
    std::vector<SurfaceData>  surfaces;
    std::vector<SourceData>   sources;
    std::vector<ReceiverData> receivers;
};

// Parámetros de configuración de la simulación. (RF-05, RF-10)
struct SimulationConfig {
    int    durationMs           = 1000;
    double soundSpeed           = 340.0;
    int    rayCount             = 642;
    int    meshSubdivisions     = 1;
    double diffusionCoefficient = 0.1;
};

// Segmento de rayo de reflexión con energía y tiempo de llegada.
struct ReflectionRaySegment {
    Vec3   from;
    Vec3   to;
    double energy = 0.0;
    int    timeMs = 0;
};

// Energía recibida por un receptor en un instante de tiempo.
struct ReceiverEnergySample {
    int    receiverId = 0;
    int    timeMs     = 0;
    double energy     = 0.0;
};

// Energía almacenada en un triángulo en un instante de tiempo.
struct TriangleEnergySample {
    int    triangleId = 0;
    int    surfaceId  = 0;
    int    timeMs     = 0;
    double energy     = 0.0;
};

// Matrices de distancia, tiempo, porcentaje y visibilidad entre todos los triángulos. (RF-03 a RF-06)
struct DiffusionMatrixData {
    std::vector<std::vector<double>> distances;
    std::vector<std::vector<int>>    timesMs;
    std::vector<std::vector<double>> percentages;
    std::vector<std::vector<bool>>   visibility;
};

// Resultado completo de la simulación. (RF-11)
struct SimulationResult {
    bool        success             = false;
    std::string message;
    std::vector<ReflectionRaySegment> reflectionRays;
    std::vector<ReceiverEnergySample> receiverEnergy;
    std::vector<TriangleEnergySample> triangleEnergy;
    DiffusionMatrixData               diffusion;
    double      totalReceiverEnergy = 0.0;
    double      lostEnergy          = 0.0;
};

} // namespace core
