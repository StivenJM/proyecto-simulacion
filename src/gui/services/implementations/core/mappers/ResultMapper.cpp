#include "ResultMapper.h"

#include "VectorMapper.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

namespace gui::coremappers {
namespace {

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

std::unordered_map<int, int> buildTrianglePlaneLookup(const GuiScenario& scenario)
{
    std::unordered_map<int, int> lookup;
    for (const GuiPlane& plane : scenario.planes) {
        for (const GuiTriangle& triangle : plane.triangles) {
            lookup[triangle.id] = plane.id;
        }
    }
    return lookup;
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

}  // namespace

SimulationResultDto toGuiResult(const core::SimulationResult& result, const GuiScenario& scenario)
{
    SimulationResultDto mapped;
    mapped.success = result.success;
    mapped.message = result.message;
    mapped.state.running = result.success;

    const auto trianglePlaneLookup = buildTrianglePlaneLookup(scenario);
    const auto planeNameLookup = buildPlaneNameLookup(scenario);

    std::unordered_map<int, float> planeEnergy;
    float maxEnergy = 0.0001f;
    for (const core::TriangleEnergySample& sample : result.triangleEnergy) {
        const auto planeIt = trianglePlaneLookup.find(sample.triangleId);
        if (planeIt == trianglePlaneLookup.end()) {
            continue;
        }

        const float energy = static_cast<float>(sample.energy);
        planeEnergy[planeIt->second] = std::max(planeEnergy[planeIt->second], energy);
        maxEnergy = std::max(maxEnergy, energy);
    }

    for (const GuiPlane& plane : scenario.planes) {
        const float normalized = clamp01(planeEnergy[plane.id] / maxEnergy);
        const auto nameIt = planeNameLookup.find(plane.id);
        mapped.planeEnergy.push_back({plane.id, nameIt != planeNameLookup.end() ? nameIt->second : "Plane " + std::to_string(plane.id), normalized});
        mapped.overlay.planeEnergy.push_back({plane.id, normalized});
    }

    for (const core::TriangleEnergySample& sample : result.triangleEnergy) {
        const auto planeIt = trianglePlaneLookup.find(sample.triangleId);
        if (planeIt != trianglePlaneLookup.end()) {
            mapped.overlay.triangleEnergy.push_back({planeIt->second, sample.triangleId, clamp01(static_cast<float>(sample.energy) / maxEnergy)});
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
        const float energy = clamp01(static_cast<float>(segment.energy));
        const float startEnergy = energy;
        const float endEnergy = energy;

        if (!continuesPreviousRay) {
            GuiSimulationRay ray;
            ray.id = nextRayId++;
            ray.activePosition = start;
            ray.activeEnergy = energy;
            ray.activeRadius = 0.018f + energy * 0.115f;
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
