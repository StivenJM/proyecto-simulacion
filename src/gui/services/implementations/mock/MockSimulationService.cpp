#include "MockSimulationService.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace gui {
namespace {

constexpr float maxSimulationSeconds = 1.0f;
constexpr float soundSpeedMetersPerSecond = 340.0f;

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

Vec3 add(Vec3 a, Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

float distance(Vec3 a, Vec3 b)
{
    const Vec3 delta = subtract(a, b);
    return std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}

Vec3 deterministicOffset(int rayIndex, int bounceIndex)
{
    const float a = static_cast<float>((rayIndex * 37 + bounceIndex * 17) % 100) / 100.0f - 0.5f;
    const float b = static_cast<float>((rayIndex * 19 + bounceIndex * 29) % 100) / 100.0f - 0.5f;
    return {a * 0.18f, b * 0.18f, (a - b) * 0.08f};
}

Vec3 triangleOrPlaneSamplePoint(const GuiPlane& plane, int rayIndex, int bounceIndex, int& triangleId)
{
    if (!plane.triangles.empty()) {
        const std::size_t triangleIndex = static_cast<std::size_t>((rayIndex + bounceIndex) % static_cast<int>(plane.triangles.size()));
        triangleId = plane.triangles[triangleIndex].id;
        return add(plane.triangles[triangleIndex].centroid, deterministicOffset(rayIndex, bounceIndex));
    }

    triangleId = -1;
    return add(plane.center, deterministicOffset(rayIndex, bounceIndex));
}

float segmentTravelTime(Vec3 start, Vec3 end)
{
    return distance(start, end) / soundSpeedMetersPerSecond;
}

float simulatedEnergyForPlane(const GuiPlane& plane, const GuiScenario& scenario)
{
    float energy = 0.0f;
    const Vec3 point = plane.triangles.empty() ? plane.center : plane.triangles.front().centroid;
    for (const GuiSource& source : scenario.sources) {
        if (!source.visible) {
            continue;
        }

        const float sourceDistance = distance(point, source.position);
        const float attenuation = 1.0f / (1.0f + sourceDistance * sourceDistance * 0.18f);
        const float absorption = 1.0f - clamp01(plane.absorption) * 0.75f;
        energy += source.energy * attenuation * absorption;
    }

    return energy;
}

}  // namespace

SimulationResultDto MockSimulationService::start(const GuiScenario& scenario)
{
    SimulationResultDto result;
    result.success = true;
    result.message = "Mock simulation completed.";
    result.state.running = true;
    result.durationSeconds = maxSimulationSeconds;

    constexpr int raysPerSource = 8;
    constexpr int maxBounces = 4;
    int nextRayId = 1;

    for (const GuiSource& source : scenario.sources) {
        if (!source.visible) {
            continue;
        }

        for (int rayIndex = 0; rayIndex < raysPerSource; ++rayIndex) {
            GuiSimulationRay ray;
            ray.id = nextRayId++;
            ray.sourceId = source.id;

            Vec3 start = source.position;
            float energy = source.energy;
            float accumulated = 0.0f;

            for (int bounceIndex = 0; bounceIndex < maxBounces; ++bounceIndex) {
                if (scenario.planes.empty()) {
                    break;
                }

                const std::size_t planeIndex = static_cast<std::size_t>((rayIndex * 2 + bounceIndex * 3) % static_cast<int>(scenario.planes.size()));
                const GuiPlane& plane = scenario.planes[planeIndex];
                if (!plane.visible) {
                    continue;
                }

                int triangleId = -1;
                const Vec3 end = triangleOrPlaneSamplePoint(plane, rayIndex, bounceIndex, triangleId);
                const float rawDuration = std::max(0.08f, segmentTravelTime(start, end) * 18.0f);
                const float endTime = std::min(maxSimulationSeconds, accumulated + rawDuration);
                const float absorptionLoss = 0.58f + clamp01(plane.absorption) * 0.24f;
                const float endEnergy = energy * absorptionLoss;

                ray.segments.push_back({start, end, plane.id, triangleId, accumulated, endTime, energy, endEnergy});
                start = end;
                accumulated = endTime;
                energy = endEnergy;

                if (accumulated >= maxSimulationSeconds || energy < 0.04f) {
                    break;
                }
            }

            if (!ray.segments.empty()) {
                ray.activePosition = ray.segments.front().start;
                ray.activeEnergy = source.energy;
                ray.activeRadius = 0.11f;
                ray.alive = true;
                result.rays.push_back(ray);
            }
        }
    }

    float maxEnergy = 0.0001f;
    std::vector<float> rawEnergy;
    rawEnergy.reserve(scenario.planes.size());
    for (const GuiPlane& plane : scenario.planes) {
        const float energy = simulatedEnergyForPlane(plane, scenario);
        rawEnergy.push_back(energy);
        maxEnergy = std::max(maxEnergy, energy);
    }

    for (std::size_t index = 0; index < scenario.planes.size(); ++index) {
        const GuiPlane& plane = scenario.planes[index];
        const float normalized = clamp01(rawEnergy[index] / maxEnergy);
        result.planeEnergy.push_back({plane.id, plane.name.empty() ? "Plane " + std::to_string(plane.id) : plane.name, normalized});
        result.overlay.planeEnergy.push_back({plane.id, normalized});
        for (const GuiTriangle& triangle : plane.triangles) {
            result.overlay.triangleEnergy.push_back({plane.id, triangle.id, normalized});
        }
    }

    result.overlay.active = true;
    result.overlay.showDiffuseEnergy = true;
    result.overlay.showRayTracing = true;
    return result;
}

}  // namespace gui
