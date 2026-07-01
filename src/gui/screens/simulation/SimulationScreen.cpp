#include "SimulationScreen.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>

namespace gui {
namespace {

constexpr float maxSimulationSeconds = 1.0f;
constexpr float minSimulationSpeedMultiplier = 0.10f;
constexpr float maxSimulationSpeedMultiplier = 4.00f;
constexpr float soundSpeedMetersPerSecond = 340.0f;

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

float distance(Vec3 a, Vec3 b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vec3 add(Vec3 a, Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 scale(Vec3 value, float factor)
{
    return {value.x * factor, value.y * factor, value.z * factor};
}

Vec3 lerp(Vec3 from, Vec3 to, float amount)
{
    return add(from, scale(subtract(to, from), amount));
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

float smoothstep(float value)
{
    const float t = clamp01(value);
    return t * t * (3.0f - 2.0f * t);
}

Vec3 planeSamplePoint(const GuiPlane& plane)
{
    if (!plane.triangles.empty()) {
        return plane.triangles.front().centroid;
    }

    return plane.center;
}

float simulatedEnergyAt(Vec3 point, const GuiPlane& plane, const GuiScenario& scenario, float progress)
{
    if (scenario.sources.empty()) {
        return 0.0f;
    }

    float energy = 0.0f;
    for (const GuiSource& source : scenario.sources) {
        if (!source.visible) {
            continue;
        }

        const float sourceDistance = distance(point, source.position);
        const float arrival = clamp01(sourceDistance / 12.0f);
        const float arrivalEnvelope = smoothstep((progress - arrival) / 0.32f);
        const float attenuation = 1.0f / (1.0f + sourceDistance * sourceDistance * 0.18f);
        const float absorption = 1.0f - clamp01(plane.absorption) * 0.75f;
        const float temporalDecay = 1.0f - progress * 0.28f;
        energy += arrivalEnvelope * attenuation * absorption * temporalDecay;
    }

    return energy;
}

const char* stateLabel(SimulationRunState state)
{
    if (state == SimulationRunState::Running) return "Running";
    if (state == SimulationRunState::Finished) return "Finished";
    return "Ready";
}

}  // namespace

bool SimulationScreen::start(const GuiScenario& scenario)
{
    if (state_ == SimulationRunState::Running) {
        return false;
    }

    elapsedSeconds_ = 0.0f;
    state_ = SimulationRunState::Running;
    generateRayPaths(scenario);
    recomputeEnergy(scenario);
    return true;
}

void SimulationScreen::restart(const GuiScenario& scenario)
{
    reset();
    start(scenario);
}

void SimulationScreen::update(float deltaTime, const GuiScenario& scenario)
{
    if (state_ != SimulationRunState::Running) {
        return;
    }

    const float simulatedDeltaTime = std::max(deltaTime, 0.0f) * simulationSpeedMultiplier_;
    elapsedSeconds_ = std::min(maxSimulationSeconds, elapsedSeconds_ + simulatedDeltaTime);
    recomputeEnergy(scenario);
    if (elapsedSeconds_ >= maxSimulationSeconds) {
        state_ = SimulationRunState::Finished;
    }
}

void SimulationScreen::renderPanel(const GuiScenario& scenario)
{
    ImGui::Begin("Simulation");
    ImGui::Text("Status: %s", stateLabel(state_));
    ImGui::Text("Time: %.2f s / %.2f s", elapsedSeconds_, maxSimulationSeconds);
    ImGui::ProgressBar(progress(), ImVec2(-1.0f, 0.0f));

    ImGui::SliderFloat("Simulation speed", &simulationSpeedMultiplier_, minSimulationSpeedMultiplier, maxSimulationSpeedMultiplier, "%.2fx");
    simulationSpeedMultiplier_ = std::clamp(simulationSpeedMultiplier_, minSimulationSpeedMultiplier, maxSimulationSpeedMultiplier);
    if (ImGui::Button("0.5x")) {
        simulationSpeedMultiplier_ = 0.5f;
    }
    ImGui::SameLine();
    if (ImGui::Button("1x")) {
        simulationSpeedMultiplier_ = 1.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("2x")) {
        simulationSpeedMultiplier_ = 2.0f;
    }

    if (state_ == SimulationRunState::Running) {
        if (ImGui::Button("Restart")) {
            restart(scenario);
        }
    } else {
        if (ImGui::Button(state_ == SimulationRunState::Finished ? "Restart" : "Start")) {
            restart(scenario);
        }
    }

    ImGui::Separator();
    if (ImGui::Checkbox("Show diffuse energy matrix on planes", &showDiffuseEnergyOnPlanes_)) {
        recomputeEnergy(scenario);
    }
    if (ImGui::Checkbox("Show ray tracing visualization", &showRayTracing_)) {
        recomputeEnergy(scenario);
    }
    ImGui::Text("Rays: %d total / %d active", rayCount(), activeRayCount());
    ImGui::TextWrapped("Ray tracing and diffuse matrix data are simulated GUI scaffolds until Core is connected.");

    ImGui::Separator();
    const int view = viewMode_ == SimulationViewMode::External ? 0 : 1;
    if (ImGui::RadioButton("External view", view == 0)) {
        viewMode_ = SimulationViewMode::External;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Internal view", view == 1)) {
        viewMode_ = SimulationViewMode::Internal;
    }

    if (!planeEnergy_.empty() && ImGui::BeginTable("PlaneEnergy", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Plane");
        ImGui::TableSetupColumn("Energy");
        ImGui::TableSetupColumn("Level");
        ImGui::TableHeadersRow();
        for (const SimulatedPlaneEnergy& sample : planeEnergy_) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(sample.planeName.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f", sample.energy);
            ImGui::TableSetColumnIndex(2);
            ImGui::ProgressBar(sample.energy, ImVec2(-1.0f, 0.0f));
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void SimulationScreen::reset()
{
    state_ = SimulationRunState::Ready;
    elapsedSeconds_ = 0.0f;
    planeEnergy_.clear();
    rays_.clear();
    overlay_ = {};
}

bool SimulationScreen::isStarted() const
{
    return state_ != SimulationRunState::Ready;
}

bool SimulationScreen::isRunning() const
{
    return state_ == SimulationRunState::Running;
}

bool SimulationScreen::isFinished() const
{
    return state_ == SimulationRunState::Finished;
}

float SimulationScreen::elapsedSeconds() const
{
    return elapsedSeconds_;
}

float SimulationScreen::progress() const
{
    return clamp01(elapsedSeconds_ / maxSimulationSeconds);
}

SimulationViewMode SimulationScreen::viewMode() const
{
    return viewMode_;
}

RenderSimulationOverlay SimulationScreen::renderOverlay() const
{
    return overlay_;
}

const std::vector<SimulatedPlaneEnergy>& SimulationScreen::planeEnergy() const
{
    return planeEnergy_;
}

int SimulationScreen::rayCount() const
{
    return static_cast<int>(rays_.size());
}

int SimulationScreen::activeRayCount() const
{
    return static_cast<int>(std::count_if(rays_.begin(), rays_.end(), [](const GuiSimulationRay& ray) {
        return ray.visible && ray.alive;
    }));
}

void SimulationScreen::generateRayPaths(const GuiScenario& scenario)
{
    rays_.clear();
    if (scenario.sources.empty() || scenario.planes.empty()) {
        return;
    }

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
            float energy = 1.0f;
            float accumulated = 0.0f;

            for (int bounceIndex = 0; bounceIndex < maxBounces; ++bounceIndex) {
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
                ray.activeEnergy = 1.0f;
                ray.activeRadius = 0.11f;
                ray.alive = true;
                rays_.push_back(ray);
            }
        }
    }
}

void SimulationScreen::recomputeEnergy(const GuiScenario& scenario)
{
    planeEnergy_.clear();
    overlay_ = {};
    overlay_.active = state_ != SimulationRunState::Ready;
    overlay_.showDiffuseEnergy = showDiffuseEnergyOnPlanes_;
    overlay_.showRayTracing = showRayTracing_;

    float maxEnergy = 0.0001f;
    std::vector<float> rawPlaneEnergy;
    rawPlaneEnergy.reserve(scenario.planes.size());

    for (const GuiPlane& plane : scenario.planes) {
        float totalEnergy = 0.0f;
        int sampleCount = 0;

        if (!plane.triangles.empty()) {
            for (const GuiTriangle& triangle : plane.triangles) {
                if (!triangle.visible) {
                    continue;
                }

                const float energy = simulatedEnergyAt(triangle.centroid, plane, scenario, progress());
                if (showDiffuseEnergyOnPlanes_) {
                    overlay_.triangleEnergy.push_back({plane.id, triangle.id, energy});
                }
                totalEnergy += energy;
                ++sampleCount;
            }
        } else {
            totalEnergy = simulatedEnergyAt(planeSamplePoint(plane), plane, scenario, progress());
            sampleCount = 1;
        }

        const float planeLevel = sampleCount > 0 ? totalEnergy / static_cast<float>(sampleCount) : 0.0f;
        rawPlaneEnergy.push_back(planeLevel);
        maxEnergy = std::max(maxEnergy, planeLevel);
    }

    for (std::size_t index = 0; index < scenario.planes.size(); ++index) {
        const GuiPlane& plane = scenario.planes[index];
        const float normalized = clamp01(rawPlaneEnergy[index] / maxEnergy);
        planeEnergy_.push_back({plane.id, plane.name.empty() ? "Plane " + std::to_string(plane.id) : plane.name, normalized});
        if (showDiffuseEnergyOnPlanes_) {
            overlay_.planeEnergy.push_back({plane.id, normalized});
        }
    }

    for (RenderTriangleEnergy& sample : overlay_.triangleEnergy) {
        sample.energy = clamp01(sample.energy / maxEnergy);
    }

    recomputeRays(progress());
}

void SimulationScreen::recomputeRays(float progressValue)
{
    if (!showRayTracing_ || state_ == SimulationRunState::Ready) {
        return;
    }

    const float currentTime = clamp01(progressValue) * maxSimulationSeconds;
    for (GuiSimulationRay& ray : rays_) {
        ray.alive = false;
        if (ray.segments.empty() || !ray.visible) {
            continue;
        }

        const GuiRaySegment* activeSegment = nullptr;
        for (const GuiRaySegment& segment : ray.segments) {
            if (currentTime >= segment.startTimeSeconds && currentTime <= segment.endTimeSeconds) {
                activeSegment = &segment;
                break;
            }
        }

        if (activeSegment == nullptr && currentTime >= ray.segments.back().endTimeSeconds) {
            activeSegment = &ray.segments.back();
        }

        if (activeSegment == nullptr) {
            continue;
        }

        const float duration = std::max(0.0001f, activeSegment->endTimeSeconds - activeSegment->startTimeSeconds);
        const float localT = clamp01((currentTime - activeSegment->startTimeSeconds) / duration);
        const float normalizedTime = clamp01(currentTime / maxSimulationSeconds);
        const float temporalDecayBase = clamp01(1.0f - normalizedTime * 0.82f);
        const float temporalDecay = std::pow(temporalDecayBase, 1.4f);
        const float segmentDiffusion = 1.0f - localT * 0.28f;
        const float segmentEnergy = activeSegment->startEnergy + (activeSegment->endEnergy - activeSegment->startEnergy) * localT;
        ray.activePosition = lerp(activeSegment->start, activeSegment->end, localT);
        ray.activeEnergy = clamp01(segmentEnergy * temporalDecay * segmentDiffusion);
        ray.activeRadius = 0.018f + ray.activeEnergy * 0.115f;
        ray.alive = ray.activeEnergy > 0.01f;

        if (ray.alive) {
            overlay_.rayParticles.push_back({ray.activePosition, ray.activeRadius, ray.activeEnergy});
        }
    }
}

}  // namespace gui
