#pragma once
#include <string>
#include <vector>

namespace services {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct TriangleData {
    int  id        = 0;
    int  surfaceId = 0;
    Vec3 a;
    Vec3 b;
    Vec3 c;
};

struct SurfaceData {
    int                       id         = 0;
    double                    absorption = 0.1;
    std::vector<TriangleData> triangles;
};

struct SourceData {
    int    id       = 0;
    Vec3   position;
    double energy   = 1.0;
};

struct ReceiverData {
    int    id       = 0;
    Vec3   position;
    double radius   = 0.5;
};

struct ScenarioData {
    std::vector<SurfaceData>  surfaces;
    std::vector<SourceData>   sources;
    std::vector<ReceiverData> receivers;
};

struct SimulationConfig {
    int    durationMs           = 1000;
    double soundSpeed           = 340.0;
    int    rayCount             = 642;
    double diffusionCoefficient = 0.5;
};

struct ReflectionRaySegment {
    Vec3   from;
    Vec3   to;
    double energy = 0.0;
    int    timeMs = 0;
};

struct ReceiverEnergySample {
    int    receiverId = 0;
    int    timeMs     = 0;
    double energy     = 0.0;
};

struct TriangleEnergySample {
    int    triangleId = 0;
    int    timeMs     = 0;
    double energy     = 0.0;
};

struct DiffusionMatrixData {
    std::vector<std::vector<double>> distances;
    std::vector<std::vector<int>>    timesMs;
    std::vector<std::vector<double>> percentages;
    std::vector<std::vector<bool>>   visibility;
};

struct SimulationResult {
    bool        success              = false;
    std::string message;
    std::vector<ReflectionRaySegment>  reflectionRays;
    std::vector<ReceiverEnergySample>  receiverEnergy;
    std::vector<TriangleEnergySample>  triangleEnergy;
    DiffusionMatrixData                diffusion;
    double      totalReceiverEnergy  = 0.0;
    double      lostEnergy           = 0.0;
};

} // namespace services
