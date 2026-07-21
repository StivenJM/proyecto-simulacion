#pragma once
#include <array>
#include <cstdint>
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
    int  gridRow   = -1;
    int  gridColumn = -1;
    int  cellId    = -1;
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

enum class DiffusionSolverMode {
    ExactDense,
    Hierarchical
};

// Parámetros de configuración de la simulación. (RF-05, RF-10)
struct SimulationConfig {
    int    durationMs           = 1000;
    double soundSpeed           = 340.0;
    int    rayCount             = 642;
    int    meshSubdivisions     = 5;
    double diffusionCoefficient = 0.1;
    double hierarchySpatialTolerance = 0.20;
    double hierarchyWeightTolerance = 0.03;
    double hierarchyDistributionTolerance = 0.10;
    double hierarchyMaximumDelaySpreadMs = 0.5;
    double hierarchyAbsorptionTolerance = 0.02;
    double diffuseCutoffDb = -70.0;
    bool useHierarchicalDiffusion = true;
    bool exportDenseDiffusionMatrices = false;
    DiffusionSolverMode diffusionSolverMode = DiffusionSolverMode::Hierarchical;
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

struct AABB {
    Vec3 min;
    Vec3 max;
};

struct DiffusionNode {
    std::uint32_t id = 0;
    std::uint32_t surfaceId = 0;
    std::int32_t parentId = -1;
    std::array<std::int32_t, 4> children{{-1, -1, -1, -1}};
    bool isLeaf = false;
    std::vector<std::uint32_t> triangleIndices;
    Vec3 centroid;
    Vec3 normal;
    AABB bounds;
    double area = 0.0;
    double absorptionMin = 0.0;
    double absorptionMax = 0.0;
    double absorptionAverage = 0.0;
    float normalDeviation = 0.0f;
    std::uint32_t depth = 0;
    std::array<float, 4> childAreaFractions{{0.0f, 0.0f, 0.0f, 0.0f}};
};

enum class VisibilityState {
    FullyVisible,
    FullyOccluded,
    Mixed,
    Unknown
};

struct HierarchicalDiffusionLink {
    std::uint32_t targetNodeId = 0;
    float rawWeight = 0.0f;
    float percentage = 0.0f;
    std::uint16_t delayBins = 1;
    float delaySpreadMs = 0.0f;
    float weightError = 0.0f;
    float distributionError = 0.0f;
    float spatialRatio = 0.0f;
    VisibilityState visibility = VisibilityState::Unknown;
};

struct HierarchicalDiffusionMetrics {
    std::uint32_t hierarchyNodeCount = 0;
    std::uint32_t hierarchyLeafCount = 0;
    std::uint32_t hierarchyMaximumDepth = 0;
    std::uint64_t hierarchicalLinkCount = 0;
    double averageLinksPerSourceTriangle = 0.0;
    std::uint32_t maximumLinksPerSourceTriangle = 0;
    std::uint64_t acceptedClusterLinkCount = 0;
    std::uint64_t exactLeafLinkCount = 0;
    std::uint64_t rejectedBySpatialRatio = 0;
    std::uint64_t rejectedByDelaySpread = 0;
    std::uint64_t rejectedByWeightError = 0;
    std::uint64_t rejectedByDistributionError = 0;
    std::uint64_t rejectedByMixedVisibility = 0;
    std::uint64_t rejectedByMaterialVariation = 0;
    std::uint64_t fullyOccludedNodeCount = 0;
    std::uint64_t sameSurfaceCulledCount = 0;
    std::uint64_t backfaceCulledCount = 0;
    std::uint64_t nodeArrivalCount = 0;
    std::uint64_t aggregatedNodeArrivalCount = 0;
    std::uint64_t nodePushDownCount = 0;
    std::uint64_t activeLeafTimeCount = 0;
    std::uint64_t hierarchicalPropagationCount = 0;
    std::uint64_t prunedCellCount = 0;
    double discardedEnergy = 0.0;
    double discardedEnergyRatio = 0.0;
    long long denseMatrixExportDurationMs = 0;
    bool denseMatrixExported = false;
};

struct HierarchicalDiffusionData {
    std::vector<DiffusionNode> nodes;
    std::vector<std::uint32_t> rootNodeIds;
    std::vector<std::vector<HierarchicalDiffusionLink>> linksBySourceTriangle;
    HierarchicalDiffusionMetrics metrics;
};

struct DiffuseEnergyResult {
    std::vector<TriangleEnergySample> samples;
    std::vector<std::vector<double>>  energyByTriangleTime;
    int                               timeStepMs = 1;
};

// Resultado completo de la simulación. (RF-11)
struct SimulationResult {
    bool        success             = false;
    std::string message;
    std::vector<ReflectionRaySegment> reflectionRays;
    std::vector<ReceiverEnergySample> receiverEnergy;
    std::vector<TriangleEnergySample> triangleEnergy;
    DiffusionMatrixData               diffusion;
    HierarchicalDiffusionData         hierarchicalDiffusion;
    std::vector<TriangleData>         diffusionTriangles;
    std::vector<std::vector<double>>  diffuseEnergyByTriangleTime;
    int                               diffuseEnergyTimeStepMs = 1;
    double      totalReceiverEnergy = 0.0;
    double      lostEnergy          = 0.0;
};

} // namespace core
