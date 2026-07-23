#include "ResultMapper.h"

#include "VectorMapper.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

namespace gui::coremappers {
namespace {

constexpr double kTriangleHitToleranceSquared = 1e-8;

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

std::unordered_map<int, std::string> buildPlaneNameLookup(const GuiScenario& scenario)
{
    std::unordered_map<int, std::string> lookup;
    for (const GuiPlane& plane : scenario.planes) {
        lookup[plane.id] = plane.name.empty() ? "Plane " + std::to_string(plane.id) : plane.name;
    }
    return lookup;
}

float distance(Vec3 a, Vec3 b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool samePoint(Vec3 a, Vec3 b)
{
    return distance(a, b) <= 0.0001f;
}

core::Vec3 subtract(core::Vec3 a, core::Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

core::Vec3 add(core::Vec3 a, core::Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

core::Vec3 scale(core::Vec3 value, double factor)
{
    return {value.x * factor, value.y * factor, value.z * factor};
}

double dot(core::Vec3 a, core::Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double lengthSquared(core::Vec3 value)
{
    return dot(value, value);
}

double pointSegmentDistanceSquared(core::Vec3 point, core::Vec3 a, core::Vec3 b)
{
    const core::Vec3 ab = subtract(b, a);
    const double denominator = lengthSquared(ab);
    if (denominator <= 1e-12) {
        return lengthSquared(subtract(point, a));
    }

    const double t = std::clamp(dot(subtract(point, a), ab) / denominator, 0.0, 1.0);
    const core::Vec3 closest = add(a, scale(ab, t));
    return lengthSquared(subtract(point, closest));
}

double pointTriangleDistanceSquared(core::Vec3 point, const core::TriangleData& triangle)
{
    const core::Vec3 ab = subtract(triangle.b, triangle.a);
    const core::Vec3 ac = subtract(triangle.c, triangle.a);
    const core::Vec3 ap = subtract(point, triangle.a);

    const double d00 = dot(ab, ab);
    const double d01 = dot(ab, ac);
    const double d11 = dot(ac, ac);
    const double d20 = dot(ap, ab);
    const double d21 = dot(ap, ac);
    const double denominator = d00 * d11 - d01 * d01;
    if (std::abs(denominator) > 1e-12) {
        const double v = (d11 * d20 - d01 * d21) / denominator;
        const double w = (d00 * d21 - d01 * d20) / denominator;
        const double u = 1.0 - v - w;
        if (u >= -1e-6 && v >= -1e-6 && w >= -1e-6) {
            const core::Vec3 projected = add(triangle.a, add(scale(ab, v), scale(ac, w)));
            return lengthSquared(subtract(point, projected));
        }
    }

    return std::min({
        pointSegmentDistanceSquared(point, triangle.a, triangle.b),
        pointSegmentDistanceSquared(point, triangle.b, triangle.c),
        pointSegmentDistanceSquared(point, triangle.c, triangle.a)
    });
}

int findHitTriangleIndex(core::Vec3 hitPoint, const std::vector<core::TriangleData>& triangles)
{
    int bestIndex = -1;
    double bestDistance = std::numeric_limits<double>::infinity();
    for (std::size_t index = 0; index < triangles.size(); ++index) {
        const double candidateDistance = pointTriangleDistanceSquared(hitPoint, triangles[index]);
        if (candidateDistance < bestDistance) {
            bestDistance = candidateDistance;
            bestIndex = static_cast<int>(index);
        }
    }

    return bestDistance <= kTriangleHitToleranceSquared ? bestIndex : -1;
}

std::unordered_map<int, double> buildPlaneAbsorptionLookup(const GuiScenario& scenario)
{
    std::unordered_map<int, double> lookup;
    for (const GuiPlane& plane : scenario.planes) {
        lookup[plane.id] = clamp01(plane.absorption);
    }
    return lookup;
}

double absorptionForPlane(const std::unordered_map<int, double>& lookup, int planeId)
{
    const auto it = lookup.find(planeId);
    return it != lookup.end() ? it->second : 0.2;
}

float millisecondsToSeconds(int timeMs)
{
    return std::max(0.0f, static_cast<float>(timeMs) / 1000.0f);
}

}  // namespace

SimulationResultDto toGuiResult(const core::SimulationResult& result, const GuiScenario& scenario)
{
    SimulationResultDto mapped;
    mapped.success = result.success;
    mapped.message = result.message;
    mapped.state.running = result.success;

    const auto planeNameLookup = buildPlaneNameLookup(scenario);
    const auto planeAbsorptionLookup = buildPlaneAbsorptionLookup(scenario);

    mapped.diffusion.distances = result.diffusion.distances;
    mapped.diffusion.timesMs = result.diffusion.timesMs;
    mapped.diffusion.percentages = result.diffusion.percentages;
    mapped.diffusion.visibility = result.diffusion.visibility;
    mapped.diffusion.energyByTriangleTime = result.diffuseEnergyByTriangleTime;
    mapped.diffusion.timeStepMs = result.diffuseEnergyTimeStepMs;
    for (const core::TriangleData& triangle : result.diffusionTriangles) {
        mapped.diffusion.triangles.push_back({triangle.id, triangle.surfaceId, {toGui(triangle.a), toGui(triangle.b), toGui(triangle.c)}});
    }

    std::unordered_map<int, float> planeEnergy;
    float maxEnergy = 0.0001f;
    for (const core::TriangleEnergySample& sample : result.triangleEnergy) {
        int planeId = sample.surfaceId;
        if (planeNameLookup.find(planeId) == planeNameLookup.end()) {
            continue;
        }

        const float energy = static_cast<float>(sample.energy);
        planeEnergy[planeId] = std::max(planeEnergy[planeId], energy);
        maxEnergy = std::max(maxEnergy, energy);
    }

    for (const GuiPlane& plane : scenario.planes) {
        const float normalized = clamp01(planeEnergy[plane.id] / maxEnergy);
        const auto nameIt = planeNameLookup.find(plane.id);
        mapped.planeEnergy.push_back({plane.id, nameIt != planeNameLookup.end() ? nameIt->second : "Plane " + std::to_string(plane.id), normalized});
        mapped.overlay.planeEnergy.push_back({plane.id, normalized});
    }

    for (const core::TriangleEnergySample& sample : result.triangleEnergy) {
        int planeId = sample.surfaceId;
        if (planeNameLookup.find(planeId) == planeNameLookup.end()) {
            continue;
        }

        mapped.overlay.triangleEnergy.push_back({planeId, sample.triangleId, clamp01(static_cast<float>(sample.energy) / maxEnergy)});
    }

    struct AbsorbedTriangleEvent {
        std::size_t triangleIndex = 0;
        int timeMs = 0;
        double energy = 0.0;
    };

    std::vector<double> absorbedEnergyByTriangle(result.diffusionTriangles.size(), 0.0);
    std::vector<AbsorbedTriangleEvent> absorbedEnergyEvents;
    for (const core::ReflectionRaySegment& segment : result.reflectionRays) {
        const int triangleIndex = findHitTriangleIndex(segment.to, result.diffusionTriangles);
        if (triangleIndex < 0) {
            continue;
        }

        const core::TriangleData& triangle = result.diffusionTriangles[static_cast<std::size_t>(triangleIndex)];
        const double absorbedEnergy = segment.energy * absorptionForPlane(planeAbsorptionLookup, triangle.surfaceId);
        if (absorbedEnergy > 0.0 && std::isfinite(absorbedEnergy)) {
            absorbedEnergyByTriangle[static_cast<std::size_t>(triangleIndex)] += absorbedEnergy;
            absorbedEnergyEvents.push_back({static_cast<std::size_t>(triangleIndex), segment.timeMs, absorbedEnergy});
        }
    }

    const std::size_t diffuseTriangleCount = std::min(result.diffusionTriangles.size(), result.diffuseEnergyByTriangleTime.size());
    for (std::size_t triangleIndex = 0; triangleIndex < diffuseTriangleCount; ++triangleIndex) {
        const double absorption = absorptionForPlane(planeAbsorptionLookup, result.diffusionTriangles[triangleIndex].surfaceId);
        for (std::size_t timeIndex = 0; timeIndex < result.diffuseEnergyByTriangleTime[triangleIndex].size(); ++timeIndex) {
            const double incomingEnergy = result.diffuseEnergyByTriangleTime[triangleIndex][timeIndex];
            const double absorbedEnergy = incomingEnergy * absorption;
            if (std::isfinite(absorbedEnergy) && absorbedEnergy > 0.0) {
                absorbedEnergyByTriangle[triangleIndex] += absorbedEnergy;
                const int timeMs = static_cast<int>(timeIndex) * std::max(1, result.diffuseEnergyTimeStepMs);
                absorbedEnergyEvents.push_back({triangleIndex, timeMs, absorbedEnergy});
            }
        }
    }

    double maxAbsorbedEnergy = 0.0001;
    for (double absorbedEnergy : absorbedEnergyByTriangle) {
        maxAbsorbedEnergy = std::max(maxAbsorbedEnergy, absorbedEnergy);
    }

    if (!mapped.diffusion.triangles.empty() && !absorbedEnergyByTriangle.empty()) {
        mapped.overlay.triangleEnergy.clear();
        for (const AbsorbedTriangleEvent& event : absorbedEnergyEvents) {
            if (event.triangleIndex >= mapped.diffusion.triangles.size()) {
                continue;
            }

            const DiffusionTriangleDto& triangle = mapped.diffusion.triangles[event.triangleIndex];
            mapped.overlay.triangleEnergy.push_back({
                triangle.planeId,
                triangle.triangleId,
                clamp01(static_cast<float>(event.energy / maxAbsorbedEnergy)),
                millisecondsToSeconds(event.timeMs),
                triangle.vertices,
                true
            });
        }
    }

    int nextRayId = 1;
    GuiSimulationRay* currentRay = nullptr;
    Vec3 previousEnd{};
    float previousEndTimeSeconds = 0.0f;

    for (const core::ReflectionRaySegment& segment : result.reflectionRays) {
        const Vec3 start = toGui(segment.from);
        const Vec3 end = toGui(segment.to);
        const bool continuesPreviousRay = currentRay != nullptr && samePoint(start, previousEnd);
        const float startTimeSeconds = continuesPreviousRay ? previousEndTimeSeconds : 0.0f;
        const float travelSeconds = std::max(0.0001f, distance(start, end) / 340.0f);
        const float endTimeSeconds = startTimeSeconds + travelSeconds;
        const float energy = static_cast<float>(segment.energy);
        const float startEnergy = energy;
        const float endEnergy = energy;

        if (!continuesPreviousRay) {
            GuiSimulationRay ray;
            ray.id = nextRayId++;
            ray.activePosition = start;
            ray.initialEnergy = energy;
            ray.activeEnergy = energy;
            ray.activeRadius = 0.133f;
            ray.alive = true;
            mapped.rays.push_back(ray);
            currentRay = &mapped.rays.back();
        }

        currentRay->segments.push_back({start, end, -1, -1, startTimeSeconds, endTimeSeconds, startEnergy, endEnergy});
        previousEnd = end;
        previousEndTimeSeconds = endTimeSeconds;
    }

    mapped.overlay.active = result.success;
    mapped.overlay.showDiffuseEnergy = true;
    mapped.overlay.showRayTracing = true;
    return mapped;
}

}  // namespace gui::coremappers
