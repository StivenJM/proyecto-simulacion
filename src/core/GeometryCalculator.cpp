#include "GeometryCalculator.h"
#include "CoreTiming.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <limits>
#include <unordered_map>

namespace core {

namespace {

Vec3 subtract(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 scale(const Vec3& v, double factor) {
    return {v.x * factor, v.y * factor, v.z * factor};
}

Vec3 add(const Vec3& a, const Vec3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

double length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

Vec3 normalize(const Vec3& v) {
    double l = length(v);
    if (l < 1e-10) return {0.0, 0.0, 0.0};
    return {v.x / l, v.y / l, v.z / l};
}

double signedPlaneDistance(const Vec3& point, const Vec3& planePoint, const Vec3& planeNormal) {
    return dot(subtract(point, planePoint), planeNormal);
}

void expandBounds(AABB& bounds, const Vec3& point) {
    bounds.min.x = std::min(bounds.min.x, point.x);
    bounds.min.y = std::min(bounds.min.y, point.y);
    bounds.min.z = std::min(bounds.min.z, point.z);
    bounds.max.x = std::max(bounds.max.x, point.x);
    bounds.max.y = std::max(bounds.max.y, point.y);
    bounds.max.z = std::max(bounds.max.z, point.z);
}

AABB emptyBounds() {
    const double inf = std::numeric_limits<double>::infinity();
    return {{inf, inf, inf}, {-inf, -inf, -inf}};
}

double distanceToAabbMin(const Vec3& point, const AABB& bounds) {
    const double dx = point.x < bounds.min.x ? bounds.min.x - point.x : (point.x > bounds.max.x ? point.x - bounds.max.x : 0.0);
    const double dy = point.y < bounds.min.y ? bounds.min.y - point.y : (point.y > bounds.max.y ? point.y - bounds.max.y : 0.0);
    const double dz = point.z < bounds.min.z ? bounds.min.z - point.z : (point.z > bounds.max.z ? point.z - bounds.max.z : 0.0);
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double distanceToAabbMax(const Vec3& point, const AABB& bounds) {
    double maxDistance = 0.0;
    for (double x : {bounds.min.x, bounds.max.x}) {
        for (double y : {bounds.min.y, bounds.max.y}) {
            for (double z : {bounds.min.z, bounds.max.z}) {
                maxDistance = std::max(maxDistance, GeometryCalculator::distance(point, {x, y, z}));
            }
        }
    }
    return maxDistance;
}

double boundingRadius(const DiffusionNode& node) {
    return distanceToAabbMax(node.centroid, node.bounds);
}

bool areCoplanar(const TriangleData& a, const TriangleData& b) {
    Vec3 na = GeometryCalculator::normalVector(a);
    Vec3 nb = GeometryCalculator::normalVector(b);
    if (length(na) < 1e-10 || length(nb) < 1e-10) return true;
    if (std::abs(dot(na, nb)) <= 0.99) return false;

    constexpr double planeEpsilon = 1e-6;
    return std::abs(signedPlaneDistance(b.a, a.a, na)) <= planeEpsilon &&
           std::abs(signedPlaneDistance(b.b, a.a, na)) <= planeEpsilon &&
           std::abs(signedPlaneDistance(b.c, a.a, na)) <= planeEpsilon;
}

double solidAngleWeight(const TriangleData& src, const TriangleData& dst) {
    Vec3 cSrc = GeometryCalculator::centroid(src);
    Vec3 cDst = GeometryCalculator::centroid(dst);
    const double d = GeometryCalculator::distance(cSrc, cDst);
    if (d < 1e-10) return 0.0;

    Vec3 dir = normalize(subtract(cDst, cSrc));
    Vec3 nSrc = GeometryCalculator::normalVector(src);
    Vec3 nDst = GeometryCalculator::normalVector(dst);

    const double cosI = std::abs(dot(nSrc, dir));
    const Vec3 negDir = scale(dir, -1.0);
    const double cosJ = std::abs(dot(nDst, negDir));
    const double areaDst = GeometryCalculator::area(dst);

    constexpr double pi = 3.14159265358979323846;
    return (cosI * cosJ * areaDst) / (pi * d * d);
}

double solidAngleWeightToNode(const TriangleData& src, const DiffusionNode& dst) {
    const Vec3 cSrc = GeometryCalculator::centroid(src);
    const double d = GeometryCalculator::distance(cSrc, dst.centroid);
    if (d < 1e-10 || dst.area <= 0.0) return 0.0;

    const Vec3 dir = normalize(subtract(dst.centroid, cSrc));
    const Vec3 nSrc = GeometryCalculator::normalVector(src);
    const double cosI = std::abs(dot(nSrc, dir));
    const double cosJ = std::abs(dot(dst.normal, scale(dir, -1.0)));

    constexpr double pi = 3.14159265358979323846;
    return (cosI * cosJ * dst.area) / (pi * d * d);
}

double surfaceAbsorption(const std::vector<SurfaceData>& surfaces, int surfaceId) {
    for (const SurfaceData& surface : surfaces) {
        if (surface.id == surfaceId) return surface.absorption;
    }
    return 0.2;
}

void finalizeNode(
    DiffusionNode& node,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>& surfaces
) {
    node.bounds = emptyBounds();
    node.area = 0.0;
    node.centroid = {};
    node.normal = {};
    node.absorptionMin = std::numeric_limits<double>::infinity();
    node.absorptionMax = -std::numeric_limits<double>::infinity();
    node.absorptionAverage = 0.0;

    for (std::uint32_t triangleIndex : node.triangleIndices) {
        const TriangleData& triangle = triangles[triangleIndex];
        const double area = GeometryCalculator::area(triangle);
        const double absorption = surfaceAbsorption(surfaces, triangle.surfaceId);
        node.area += area;
        node.centroid = add(node.centroid, scale(GeometryCalculator::centroid(triangle), area));
        node.normal = add(node.normal, scale(GeometryCalculator::normalVector(triangle), area));
        node.absorptionMin = std::min(node.absorptionMin, absorption);
        node.absorptionMax = std::max(node.absorptionMax, absorption);
        node.absorptionAverage += absorption * area;
        expandBounds(node.bounds, triangle.a);
        expandBounds(node.bounds, triangle.b);
        expandBounds(node.bounds, triangle.c);
    }

    if (node.area > 1e-12) {
        node.centroid = scale(node.centroid, 1.0 / node.area);
        node.normal = normalize(node.normal);
        node.absorptionAverage /= node.area;
    }
    if (node.triangleIndices.empty()) {
        node.absorptionMin = 0.0;
        node.absorptionMax = 0.0;
    }
}

std::uint32_t createNode(
    HierarchicalDiffusionData& data,
    int surfaceId,
    int parentId,
    std::uint32_t depth,
    std::vector<std::uint32_t> triangleIndices,
    bool isLeaf,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>& surfaces
) {
    DiffusionNode node;
    node.id = static_cast<std::uint32_t>(data.nodes.size());
    node.surfaceId = static_cast<std::uint32_t>(std::max(0, surfaceId));
    node.parentId = parentId;
    node.depth = depth;
    node.triangleIndices = std::move(triangleIndices);
    node.isLeaf = isLeaf;
    finalizeNode(node, triangles, surfaces);
    data.metrics.hierarchyMaximumDepth = std::max(data.metrics.hierarchyMaximumDepth, depth);
    if (isLeaf) ++data.metrics.hierarchyLeafCount;
    data.nodes.push_back(std::move(node));
    return static_cast<std::uint32_t>(data.nodes.size() - 1);
}

std::uint32_t buildGridNode(
    HierarchicalDiffusionData& data,
    int surfaceId,
    int parentId,
    std::uint32_t depth,
    int rowBegin,
    int rowEnd,
    int columnBegin,
    int columnEnd,
    const std::vector<std::uint32_t>& surfaceTriangles,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>& surfaces
) {
    std::vector<std::uint32_t> contained;
    for (std::uint32_t index : surfaceTriangles) {
        const TriangleData& triangle = triangles[index];
        if (triangle.gridRow >= rowBegin && triangle.gridRow < rowEnd &&
            triangle.gridColumn >= columnBegin && triangle.gridColumn < columnEnd) {
            contained.push_back(index);
        }
    }

    const bool leaf = (rowEnd - rowBegin <= 1 && columnEnd - columnBegin <= 1) || contained.size() <= 2;
    const std::uint32_t nodeId = createNode(data, surfaceId, parentId, depth, contained, leaf, triangles, surfaces);
    if (leaf) return nodeId;

    DiffusionNode& node = data.nodes[nodeId];
    const int rowMid = rowBegin + std::max(1, (rowEnd - rowBegin) / 2);
    const int colMid = columnBegin + std::max(1, (columnEnd - columnBegin) / 2);
    const int ranges[4][4] = {
        {rowBegin, rowMid, columnBegin, colMid},
        {rowBegin, rowMid, colMid, columnEnd},
        {rowMid, rowEnd, columnBegin, colMid},
        {rowMid, rowEnd, colMid, columnEnd}
    };
    for (int childSlot = 0; childSlot < 4; ++childSlot) {
        if (ranges[childSlot][0] >= ranges[childSlot][1] || ranges[childSlot][2] >= ranges[childSlot][3]) continue;
        std::uint32_t childId = buildGridNode(data, surfaceId, static_cast<int>(nodeId), depth + 1,
            ranges[childSlot][0], ranges[childSlot][1], ranges[childSlot][2], ranges[childSlot][3],
            surfaceTriangles, triangles, surfaces);
        data.nodes[nodeId].children[childSlot] = static_cast<std::int32_t>(childId);
    }
    for (int childSlot = 0; childSlot < 4; ++childSlot) {
        const int childId = data.nodes[nodeId].children[childSlot];
        if (childId >= 0 && data.nodes[nodeId].area > 1e-12) {
            data.nodes[nodeId].childAreaFractions[childSlot] = static_cast<float>(data.nodes[childId].area / data.nodes[nodeId].area);
        }
    }
    return nodeId;
}

std::uint32_t buildListNode(
    HierarchicalDiffusionData& data,
    int surfaceId,
    int parentId,
    std::uint32_t depth,
    std::vector<std::uint32_t> indices,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>& surfaces
) {
    const bool leaf = indices.size() <= 2;
    const std::uint32_t nodeId = createNode(data, surfaceId, parentId, depth, indices, leaf, triangles, surfaces);
    if (leaf) return nodeId;

    std::sort(indices.begin(), indices.end(), [&](std::uint32_t lhs, std::uint32_t rhs) {
        return GeometryCalculator::centroid(triangles[lhs]).x < GeometryCalculator::centroid(triangles[rhs]).x;
    });
    const std::size_t mid = indices.size() / 2;
    std::vector<std::uint32_t> left(indices.begin(), indices.begin() + static_cast<std::ptrdiff_t>(mid));
    std::vector<std::uint32_t> right(indices.begin() + static_cast<std::ptrdiff_t>(mid), indices.end());
    data.nodes[nodeId].children[0] = static_cast<std::int32_t>(buildListNode(data, surfaceId, static_cast<int>(nodeId), depth + 1, std::move(left), triangles, surfaces));
    data.nodes[nodeId].children[1] = static_cast<std::int32_t>(buildListNode(data, surfaceId, static_cast<int>(nodeId), depth + 1, std::move(right), triangles, surfaces));
    for (int childSlot = 0; childSlot < 4; ++childSlot) {
        const int childId = data.nodes[nodeId].children[childSlot];
        if (childId >= 0 && data.nodes[nodeId].area > 1e-12) {
            data.nodes[nodeId].childAreaFractions[childSlot] = static_cast<float>(data.nodes[childId].area / data.nodes[nodeId].area);
        }
    }
    return nodeId;
}

bool sameCoplanarSurface(const TriangleData& source, const DiffusionNode& target, const std::vector<TriangleData>& triangles) {
    if (source.surfaceId != static_cast<int>(target.surfaceId)) return false;
    for (std::uint32_t triangleIndex : target.triangleIndices) {
        if (!areCoplanar(source, triangles[triangleIndex])) return false;
    }
    return true;
}

struct LinkEstimate {
    double rawWeight = 0.0;
    int delayBins = 1;
    double delaySpreadMs = 0.0;
    double weightError = 0.0;
    double distributionError = 0.0;
    double spatialRatio = 0.0;
    VisibilityState visibility = VisibilityState::FullyVisible;
};

LinkEstimate estimateInteraction(
    const TriangleData& source,
    const DiffusionNode& target,
    const HierarchicalDiffusionData& data,
    const SimulationConfig& config
) {
    LinkEstimate estimate;
    const Vec3 sourceCentroid = GeometryCalculator::centroid(source);
    const double distance = GeometryCalculator::distance(sourceCentroid, target.centroid);
    estimate.rawWeight = solidAngleWeightToNode(source, target);
    estimate.spatialRatio = distance > 1e-10 ? boundingRadius(target) / distance : std::numeric_limits<double>::infinity();

    const double minDistance = distanceToAabbMin(sourceCentroid, target.bounds);
    const double maxDistance = distanceToAabbMax(sourceCentroid, target.bounds);
    if (config.soundSpeed > 0.0) {
        estimate.delaySpreadMs = ((maxDistance - minDistance) / config.soundSpeed) * 1000.0;
        estimate.delayBins = std::max(1, static_cast<int>(std::ceil((distance / config.soundSpeed) * 1000.0)));
    }

    if (!target.isLeaf) {
        double childWeightSum = 0.0;
        double childWeights[4] = {0.0, 0.0, 0.0, 0.0};
        for (int childSlot = 0; childSlot < 4; ++childSlot) {
            const int childId = target.children[childSlot];
            if (childId < 0) continue;
            childWeights[childSlot] = solidAngleWeightToNode(source, data.nodes[childId]);
            childWeightSum += childWeights[childSlot];
        }
        if (childWeightSum > 1e-12) {
            estimate.weightError = std::abs(estimate.rawWeight - childWeightSum) / childWeightSum;
            for (int childSlot = 0; childSlot < 4; ++childSlot) {
                const int childId = target.children[childSlot];
                if (childId < 0) continue;
                const double transferFraction = childWeights[childSlot] / childWeightSum;
                estimate.distributionError = std::max(
                    estimate.distributionError,
                    std::abs(transferFraction - static_cast<double>(target.childAreaFractions[childSlot]))
                );
            }
        }
    }
    return estimate;
}

HierarchicalDiffusionLink makeLink(std::uint32_t targetNodeId, const LinkEstimate& estimate) {
    HierarchicalDiffusionLink link;
    link.targetNodeId = targetNodeId;
    link.rawWeight = static_cast<float>(std::max(0.0, estimate.rawWeight));
    link.delayBins = static_cast<std::uint16_t>(std::max(1, std::min(65535, estimate.delayBins)));
    link.delaySpreadMs = static_cast<float>(estimate.delaySpreadMs);
    link.weightError = static_cast<float>(estimate.weightError);
    link.distributionError = static_cast<float>(estimate.distributionError);
    link.spatialRatio = static_cast<float>(estimate.spatialRatio);
    link.visibility = estimate.visibility;
    return link;
}

void refineTarget(
    std::uint32_t sourceTriangleIndex,
    std::uint32_t targetNodeId,
    HierarchicalDiffusionData& data,
    const std::vector<TriangleData>& triangles,
    const SimulationConfig& config,
    std::vector<HierarchicalDiffusionLink>& output
) {
    const TriangleData& source = triangles[sourceTriangleIndex];
    const DiffusionNode& target = data.nodes[targetNodeId];

    if (sameCoplanarSurface(source, target, triangles)) {
        ++data.metrics.sameSurfaceCulledCount;
        ++data.metrics.fullyOccludedNodeCount;
        return;
    }

    LinkEstimate estimate = estimateInteraction(source, target, data, config);
    if (estimate.rawWeight <= 1e-12) {
        ++data.metrics.backfaceCulledCount;
        return;
    }

    const bool materialOk = (target.absorptionMax - target.absorptionMin) <= config.hierarchyAbsorptionTolerance;
    const bool spatialOk = estimate.spatialRatio <= config.hierarchySpatialTolerance;
    const bool delayOk = estimate.delaySpreadMs <= config.hierarchyMaximumDelaySpreadMs;
    const bool weightOk = estimate.weightError <= config.hierarchyWeightTolerance;
    const bool distributionOk = estimate.distributionError <= config.hierarchyDistributionTolerance;

    if (!target.isLeaf && materialOk && spatialOk && delayOk && weightOk && distributionOk) {
        output.push_back(makeLink(targetNodeId, estimate));
        ++data.metrics.acceptedClusterLinkCount;
        return;
    }

    if (target.isLeaf) {
        output.push_back(makeLink(targetNodeId, estimate));
        ++data.metrics.exactLeafLinkCount;
        return;
    }

    if (!materialOk) ++data.metrics.rejectedByMaterialVariation;
    if (!spatialOk) ++data.metrics.rejectedBySpatialRatio;
    if (!delayOk) ++data.metrics.rejectedByDelaySpread;
    if (!weightOk) ++data.metrics.rejectedByWeightError;
    if (!distributionOk) ++data.metrics.rejectedByDistributionError;

    for (int childId : target.children) {
        if (childId >= 0) {
            refineTarget(sourceTriangleIndex, static_cast<std::uint32_t>(childId), data, triangles, config, output);
        }
    }
}

} // namespace

Vec3 GeometryCalculator::centroid(const TriangleData& tri) {
    return {
        (tri.a.x + tri.b.x + tri.c.x) / 3.0,
        (tri.a.y + tri.b.y + tri.c.y) / 3.0,
        (tri.a.z + tri.b.z + tri.c.z) / 3.0
    };
}

double GeometryCalculator::area(const TriangleData& tri) {
    Vec3 ab = subtract(tri.b, tri.a);
    Vec3 ac = subtract(tri.c, tri.a);
    return length(cross(ab, ac)) / 2.0;
}

Vec3 GeometryCalculator::normalVector(const TriangleData& tri) {
    Vec3 ab = subtract(tri.b, tri.a);
    Vec3 ac = subtract(tri.c, tri.a);
    return normalize(cross(ab, ac));
}

double GeometryCalculator::distance(const Vec3& a, const Vec3& b) {
    return length(subtract(b, a));
}

int GeometryCalculator::timeOfFlightMs(double distanceMeters, double soundSpeed) {
    if (soundSpeed <= 0.0) return 0;
    return static_cast<int>(std::lround((distanceMeters / soundSpeed) * 1000.0));
}

bool GeometryCalculator::areVisible(const TriangleData& a, const TriangleData& b) {
    // Triangulos coplanares no son visibles entre si (RF-04)
    return !areCoplanar(a, b);
}

DiffusionMatrixData GeometryCalculator::buildDiffusionMatrix(
    const std::vector<TriangleData>& triangles,
    double soundSpeed
) {
    const auto start = std::chrono::steady_clock::now();
    int n = static_cast<int>(triangles.size());
    const long long pairCount = static_cast<long long>(n) * static_cast<long long>(std::max(0, n - 1));
    long long visiblePairCount = 0;
    DiffusionMatrixData result;

    result.distances.assign(n, std::vector<double>(n, 0.0));
    result.timesMs.assign(n, std::vector<int>(n, 0));
    result.percentages.assign(n, std::vector<double>(n, 0.0));
    result.visibility.assign(n, std::vector<bool>(n, false));

    for (int i = 0; i < n; i++) {
        Vec3 ci = centroid(triangles[i]);
        std::vector<double> weights(n, 0.0);
        double totalWeight = 0.0;

        for (int j = 0; j < n; j++) {
            if (i == j) continue;

            Vec3   cj   = centroid(triangles[j]);
            double dist = distance(ci, cj);

            result.distances[i][j]  = dist;
            result.timesMs[i][j]    = timeOfFlightMs(dist, soundSpeed);
            result.visibility[i][j] = areVisible(triangles[i], triangles[j]);

            if (result.visibility[i][j]) {
                ++visiblePairCount;
                weights[j] = solidAngleWeight(triangles[i], triangles[j]);
                totalWeight += weights[j];
            }
        }

        // RF-06: distribucion ponderada por angulo solido entre triangulos visibles.
        if (totalWeight > 1e-12) {
            for (int j = 0; j < n; j++) {
                if (i != j && result.visibility[i][j]) {
                    result.percentages[i][j] = weights[j] / totalWeight;
                }
            }
        }
    }

    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    coreTiming() << "[CoreTiming] GeometryCalculator::buildDiffusionMatrix durationMs="
              << durationMs
              << " triangleCount=" << n
              << " pairCount=" << pairCount
              << " visiblePairCount=" << visiblePairCount << "\n";

    return result;
}

HierarchicalDiffusionData GeometryCalculator::buildHierarchicalDiffusionData(
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>& surfaces,
    const SimulationConfig& config
) {
    const auto start = std::chrono::steady_clock::now();
    HierarchicalDiffusionData data;
    data.linksBySourceTriangle.assign(triangles.size(), {});

    std::unordered_map<int, std::vector<std::uint32_t>> trianglesBySurface;
    for (std::uint32_t index = 0; index < triangles.size(); ++index) {
        trianglesBySurface[triangles[index].surfaceId].push_back(index);
    }

    for (const auto& entry : trianglesBySurface) {
        const int surfaceId = entry.first;
        const std::vector<std::uint32_t>& surfaceTriangles = entry.second;
        if (surfaceTriangles.empty()) continue;

        bool hasGrid = true;
        int maxRow = -1;
        int maxColumn = -1;
        for (std::uint32_t index : surfaceTriangles) {
            hasGrid = hasGrid && triangles[index].gridRow >= 0 && triangles[index].gridColumn >= 0;
            maxRow = std::max(maxRow, triangles[index].gridRow);
            maxColumn = std::max(maxColumn, triangles[index].gridColumn);
        }

        std::uint32_t rootId = 0;
        if (hasGrid && maxRow >= 0 && maxColumn >= 0) {
            rootId = buildGridNode(data, surfaceId, -1, 0, 0, maxRow + 1, 0, maxColumn + 1, surfaceTriangles, triangles, surfaces);
        } else {
            rootId = buildListNode(data, surfaceId, -1, 0, surfaceTriangles, triangles, surfaces);
        }
        data.rootNodeIds.push_back(rootId);
    }

    for (std::uint32_t sourceIndex = 0; sourceIndex < triangles.size(); ++sourceIndex) {
        std::vector<HierarchicalDiffusionLink>& links = data.linksBySourceTriangle[sourceIndex];
        for (std::uint32_t rootId : data.rootNodeIds) {
            refineTarget(sourceIndex, rootId, data, triangles, config, links);
        }

        double totalWeight = 0.0;
        for (const HierarchicalDiffusionLink& link : links) totalWeight += link.rawWeight;
        if (totalWeight > 1e-12) {
            for (HierarchicalDiffusionLink& link : links) {
                link.percentage = static_cast<float>(static_cast<double>(link.rawWeight) / totalWeight);
            }
        } else {
            links.clear();
        }

        data.metrics.hierarchicalLinkCount += links.size();
        data.metrics.maximumLinksPerSourceTriangle = std::max(
            data.metrics.maximumLinksPerSourceTriangle,
            static_cast<std::uint32_t>(links.size())
        );
    }

    data.metrics.hierarchyNodeCount = static_cast<std::uint32_t>(data.nodes.size());
    data.metrics.averageLinksPerSourceTriangle = triangles.empty()
        ? 0.0
        : static_cast<double>(data.metrics.hierarchicalLinkCount) / static_cast<double>(triangles.size());

    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    coreTiming() << "[CoreTiming] GeometryCalculator::buildHierarchicalDiffusionData durationMs="
              << durationMs
              << " triangleCount=" << triangles.size()
              << " nodeCount=" << data.metrics.hierarchyNodeCount
              << " leafCount=" << data.metrics.hierarchyLeafCount
              << " maxDepth=" << data.metrics.hierarchyMaximumDepth
              << " linkCount=" << data.metrics.hierarchicalLinkCount
              << " averageLinksPerTriangle=" << data.metrics.averageLinksPerSourceTriangle
              << " maxLinksPerTriangle=" << data.metrics.maximumLinksPerSourceTriangle
              << " acceptedClusterLinkCount=" << data.metrics.acceptedClusterLinkCount
              << " exactLeafLinkCount=" << data.metrics.exactLeafLinkCount
              << " sameSurfaceCulledCount=" << data.metrics.sameSurfaceCulledCount
              << " backfaceCulledCount=" << data.metrics.backfaceCulledCount << "\n";

    return data;
}

} // namespace core
