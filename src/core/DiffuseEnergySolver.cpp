#include "DiffuseEnergySolver.h"
#include "CoreTiming.h"
#include "GeometryCalculator.h"
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <unordered_map>

namespace core {

std::vector<TriangleEnergySample> DiffuseEnergySolver::solve(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    return solveDetailed(diffusion, triangles, surfaces, initialSeeds, config).samples;
}

DiffuseEnergyResult DiffuseEnergySolver::solveDetailed(
    const DiffusionMatrixData&       diffusion,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    const auto start = std::chrono::steady_clock::now();
    DiffuseEnergyResult result;
    std::vector<TriangleEnergySample> samples;

    int n = static_cast<int>(triangles.size());
    if (n == 0) {
        coreTiming() << "[CoreTiming] DiffuseEnergySolver::solveDetailed durationMs=0"
                  << " triangleCount=0"
                  << " simulationDurationMs=" << std::max(0, std::min(config.durationMs, 1000))
                  << " seedCount=" << initialSeeds.size()
                  << " acceptedSeedCount=0"
                  << " sampleCount=0"
                  << " activeCellCount=0"
                  << " propagationCount=0\n";
        return result;
    }

    const int durationMs = std::max(0, std::min(config.durationMs, 1000));
    std::size_t acceptedSeedCount = 0;
    long long activeCellCount = 0;
    long long propagationCount = 0;
    std::vector<std::vector<double>> arrivals(
        static_cast<std::size_t>(durationMs + 1),
        std::vector<double>(static_cast<std::size_t>(n), 0.0)
    );
    result.energyByTriangleTime.assign(
        static_cast<std::size_t>(n),
        std::vector<double>(static_cast<std::size_t>(durationMs + 1), 0.0)
    );

    std::unordered_map<int, int> triangleIndexById;
    std::vector<double> absorptionByTriangle(static_cast<std::size_t>(n), 0.2);
    for (int i = 0; i < n; ++i) triangleIndexById[triangles[i].id] = i;

    for (const auto& surf : surfaces) {
        for (const auto& tri : surf.triangles) {
            auto it = triangleIndexById.find(tri.id);
            if (it != triangleIndexById.end()) {
                absorptionByTriangle[static_cast<std::size_t>(it->second)] = surf.absorption;
            }
        }
    }

    for (const auto& seed : initialSeeds) {
        auto it = triangleIndexById.find(seed.triangleId);
        if (it == triangleIndexById.end()) continue;
        if (seed.timeMs < 0 || seed.timeMs > durationMs) continue;
        if (seed.energy <= 0.0) continue;
        arrivals[seed.timeMs][it->second] += seed.energy;
        ++acceptedSeedCount;
    }

    for (int timeMs = 0; timeMs <= durationMs; ++timeMs) {
        std::vector<double>& energy = arrivals[timeMs];
        double total = std::accumulate(energy.begin(), energy.end(), 0.0);
        if (total < 1e-9) continue;

        for (int i = 0; i < n; i++) {
            if (energy[i] < 1e-10) continue;
            ++activeCellCount;

            samples.push_back({triangles[i].id, triangles[i].surfaceId, timeMs, energy[i]});
            result.energyByTriangleTime[static_cast<std::size_t>(i)][static_cast<std::size_t>(timeMs)] += energy[i];

            // RF-08: aplicar absorcion de la superficie
            double afterAbsorb = energy[i] * (1.0 - absorptionByTriangle[static_cast<std::size_t>(i)]);
            if (std::abs(afterAbsorb) < 1e-10) continue;

            // RF-07: distribuir hacia triangulos visibles respetando tiempos (RF-05) y porcentajes (RF-06)
            for (int j = 0; j < n; j++) {
                if (i == j || !diffusion.visibility[i][j]) continue;
                const int arrivalMs = timeMs + diffusion.timesMs[i][j];
                if (arrivalMs > durationMs) continue;
                arrivals[arrivalMs][j] += afterAbsorb * diffusion.percentages[i][j];
                ++propagationCount;
            }
        }
    }

    result.samples = std::move(samples);
    const auto solveDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    coreTiming() << "[CoreTiming] DiffuseEnergySolver::solveDetailed durationMs="
              << solveDurationMs
              << " triangleCount=" << n
              << " simulationDurationMs=" << durationMs
              << " seedCount=" << initialSeeds.size()
              << " acceptedSeedCount=" << acceptedSeedCount
              << " sampleCount=" << result.samples.size()
              << " activeCellCount=" << activeCellCount
              << " propagationCount=" << propagationCount << "\n";
    return result;
}

DiffuseEnergyResult DiffuseEnergySolver::solveHierarchical(
    HierarchicalDiffusionData&       hierarchy,
    const std::vector<TriangleData>& triangles,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<TriangleEnergySample>& initialSeeds,
    const SimulationConfig&          config
) {
    const auto start = std::chrono::steady_clock::now();
    DiffuseEnergyResult result;
    std::vector<TriangleEnergySample> samples;

    const int triangleCount = static_cast<int>(triangles.size());
    const int nodeCount = static_cast<int>(hierarchy.nodes.size());
    const int durationMs = std::max(0, std::min(config.durationMs, 1000));
    if (triangleCount == 0 || nodeCount == 0) return result;

    result.energyByTriangleTime.assign(
        static_cast<std::size_t>(triangleCount),
        std::vector<double>(static_cast<std::size_t>(durationMs + 1), 0.0)
    );

    std::unordered_map<int, int> triangleIndexById;
    std::vector<double> absorptionByTriangle(static_cast<std::size_t>(triangleCount), 0.2);
    std::vector<double> areaByTriangle(static_cast<std::size_t>(triangleCount), 0.0);
    for (int i = 0; i < triangleCount; ++i) {
        triangleIndexById[triangles[i].id] = i;
        areaByTriangle[static_cast<std::size_t>(i)] = GeometryCalculator::area(triangles[i]);
    }

    for (const auto& surf : surfaces) {
        for (const auto& tri : surf.triangles) {
            auto it = triangleIndexById.find(tri.id);
            if (it != triangleIndexById.end()) {
                absorptionByTriangle[static_cast<std::size_t>(it->second)] = surf.absorption;
            }
        }
    }

    const std::size_t timeCount = static_cast<std::size_t>(durationMs + 1);
    const std::size_t totalNodeCells = timeCount * static_cast<std::size_t>(nodeCount);
    const std::size_t totalLeafCells = timeCount * static_cast<std::size_t>(triangleCount);
    std::vector<double> nodeArrivals(totalNodeCells, 0.0);
    std::vector<double> leafArrivals(totalLeafCells, 0.0);
    std::vector<unsigned char> nodeScheduled(totalNodeCells, 0);
    std::vector<unsigned char> leafScheduled(totalLeafCells, 0);
    std::vector<std::vector<std::uint32_t>> activeNodesAtTime(timeCount);
    std::vector<std::vector<std::uint32_t>> activeLeavesAtTime(timeCount);

    auto nodeCellIndex = [nodeCount](int timeMs, std::uint32_t nodeId) {
        return static_cast<std::size_t>(timeMs) * static_cast<std::size_t>(nodeCount) + static_cast<std::size_t>(nodeId);
    };
    auto leafCellIndex = [triangleCount](int timeMs, std::uint32_t triangleIndex) {
        return static_cast<std::size_t>(timeMs) * static_cast<std::size_t>(triangleCount) + static_cast<std::size_t>(triangleIndex);
    };

    auto addNodeArrival = [&](int timeMs, std::uint32_t nodeId, double energy) {
        if (timeMs < 0 || timeMs > durationMs || energy <= 0.0) return;
        const std::size_t index = nodeCellIndex(timeMs, nodeId);
        if (nodeScheduled[index]) {
            hierarchy.metrics.aggregatedNodeArrivalCount++;
        } else {
            nodeScheduled[index] = 1;
            activeNodesAtTime[static_cast<std::size_t>(timeMs)].push_back(nodeId);
            hierarchy.metrics.nodeArrivalCount++;
        }
        nodeArrivals[index] += energy;
    };

    auto addLeafArrival = [&](int timeMs, std::uint32_t triangleIndex, double energy) {
        if (timeMs < 0 || timeMs > durationMs || energy <= 0.0) return;
        const std::size_t index = leafCellIndex(timeMs, triangleIndex);
        if (!leafScheduled[index]) {
            leafScheduled[index] = 1;
            activeLeavesAtTime[static_cast<std::size_t>(timeMs)].push_back(triangleIndex);
            hierarchy.metrics.activeLeafTimeCount++;
        }
        leafArrivals[index] += energy;
    };

    std::size_t acceptedSeedCount = 0;
    double seedEnergy = 0.0;
    for (const auto& seed : initialSeeds) {
        auto it = triangleIndexById.find(seed.triangleId);
        if (it == triangleIndexById.end()) continue;
        if (seed.timeMs < 0 || seed.timeMs > durationMs) continue;
        if (seed.energy <= 0.0) continue;
        addLeafArrival(seed.timeMs, static_cast<std::uint32_t>(it->second), seed.energy);
        ++acceptedSeedCount;
        seedEnergy += seed.energy;
    }

    const double cutoffEnergy = std::pow(10.0, config.diffuseCutoffDb / 10.0);
    double energyOutsideHorizon = 0.0;

    for (int timeMs = 0; timeMs <= durationMs; ++timeMs) {
        auto& activeNodes = activeNodesAtTime[static_cast<std::size_t>(timeMs)];
        std::sort(activeNodes.begin(), activeNodes.end(), [&](std::uint32_t lhs, std::uint32_t rhs) {
            return hierarchy.nodes[lhs].depth < hierarchy.nodes[rhs].depth;
        });

        for (std::size_t cursor = 0; cursor < activeNodes.size(); ++cursor) {
            const std::uint32_t nodeId = activeNodes[cursor];
            const std::size_t index = nodeCellIndex(timeMs, nodeId);
            const double energy = nodeArrivals[index];
            nodeArrivals[index] = 0.0;
            nodeScheduled[index] = 0;
            if (energy <= 0.0) continue;

            const DiffusionNode& node = hierarchy.nodes[nodeId];
            if (node.isLeaf) {
                double totalArea = 0.0;
                for (std::uint32_t triangleIndex : node.triangleIndices) totalArea += areaByTriangle[triangleIndex];
                if (totalArea <= 1e-12) continue;
                for (std::uint32_t triangleIndex : node.triangleIndices) {
                    addLeafArrival(timeMs, triangleIndex, energy * areaByTriangle[triangleIndex] / totalArea);
                }
                continue;
            }

            for (int childSlot = 0; childSlot < 4; ++childSlot) {
                const int childId = node.children[childSlot];
                if (childId < 0) continue;
                const double childEnergy = energy * static_cast<double>(node.childAreaFractions[childSlot]);
                addNodeArrival(timeMs, static_cast<std::uint32_t>(childId), childEnergy);
                hierarchy.metrics.nodePushDownCount++;
            }
        }

        auto& activeLeaves = activeLeavesAtTime[static_cast<std::size_t>(timeMs)];
        for (std::uint32_t triangleIndex : activeLeaves) {
            const std::size_t index = leafCellIndex(timeMs, triangleIndex);
            const double incomingEnergy = leafArrivals[index];
            leafArrivals[index] = 0.0;
            leafScheduled[index] = 0;
            if (incomingEnergy <= 1e-10) continue;

            samples.push_back({triangles[triangleIndex].id, triangles[triangleIndex].surfaceId, timeMs, incomingEnergy});
            result.energyByTriangleTime[triangleIndex][static_cast<std::size_t>(timeMs)] += incomingEnergy;

            const double reflectedEnergy = incomingEnergy * (1.0 - absorptionByTriangle[triangleIndex]);
            if (reflectedEnergy < cutoffEnergy) {
                hierarchy.metrics.discardedEnergy += reflectedEnergy;
                hierarchy.metrics.prunedCellCount++;
                continue;
            }

            const std::vector<HierarchicalDiffusionLink>& links = hierarchy.linksBySourceTriangle[triangleIndex];
            for (const HierarchicalDiffusionLink& link : links) {
                const int arrivalTime = timeMs + static_cast<int>(link.delayBins);
                const double transferred = reflectedEnergy * static_cast<double>(link.percentage);
                if (transferred < cutoffEnergy) {
                    hierarchy.metrics.discardedEnergy += transferred;
                    hierarchy.metrics.prunedCellCount++;
                    continue;
                }
                if (arrivalTime > durationMs) {
                    energyOutsideHorizon += transferred;
                    continue;
                }
                addNodeArrival(arrivalTime, link.targetNodeId, transferred);
                hierarchy.metrics.hierarchicalPropagationCount++;
            }
        }
    }

    if (seedEnergy > 1e-12) {
        hierarchy.metrics.discardedEnergyRatio = hierarchy.metrics.discardedEnergy / seedEnergy;
    }

    result.samples = std::move(samples);
    const auto solveDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();
    coreTiming() << "[CoreTiming] DiffuseEnergySolver::solveHierarchical durationMs="
              << solveDurationMs
              << " triangleCount=" << triangleCount
              << " nodeCount=" << nodeCount
              << " simulationDurationMs=" << durationMs
              << " seedCount=" << initialSeeds.size()
              << " acceptedSeedCount=" << acceptedSeedCount
              << " sampleCount=" << result.samples.size()
              << " nodeArrivalCount=" << hierarchy.metrics.nodeArrivalCount
              << " aggregatedNodeArrivalCount=" << hierarchy.metrics.aggregatedNodeArrivalCount
              << " nodePushDownCount=" << hierarchy.metrics.nodePushDownCount
              << " activeLeafTimeCount=" << hierarchy.metrics.activeLeafTimeCount
              << " hierarchicalPropagationCount=" << hierarchy.metrics.hierarchicalPropagationCount
              << " prunedCellCount=" << hierarchy.metrics.prunedCellCount
              << " discardedEnergy=" << hierarchy.metrics.discardedEnergy
              << " energyOutsideHorizon=" << energyOutsideHorizon << "\n";
    return result;
}

} // namespace core
