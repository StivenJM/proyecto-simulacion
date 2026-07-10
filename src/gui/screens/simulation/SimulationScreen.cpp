#include "SimulationScreen.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>

namespace gui {
namespace {

constexpr float maxSimulationSeconds = 1.0f;
constexpr float minSimulationSpeedMultiplier = 0.001f;
constexpr float maxSimulationSpeedMultiplier = 2.00f;
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

SimulationScreen::SimulationScreen(ISimulationService& simulationService)
    : simulationService_(simulationService)
{
}

bool SimulationScreen::start(const GuiScenario& scenario)
{
    if (state_ == SimulationRunState::Running) {
        return false;
    }

    elapsedSeconds_ = 0.0f;
    const SimulationResultDto result = simulationService_.start(scenario);
    planeEnergy_.clear();
    planeEnergy_.reserve(result.planeEnergy.size());
    for (const SimulationPlaneEnergyDto& sample : result.planeEnergy) {
        planeEnergy_.push_back({sample.planeId, sample.planeName, sample.energy});
    }
    rays_ = result.rays;
    resultOverlay_ = result.overlay;
    statusMessage_ = result.message;
    configuredRayCount_ = result.configuredRayCount;
    state_ = result.success ? SimulationRunState::Running : SimulationRunState::Ready;
    recomputeOverlay();
    return result.success;
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
    recomputeOverlay();
    if (elapsedSeconds_ >= maxSimulationSeconds) {
        state_ = SimulationRunState::Finished;
    }
}

void SimulationScreen::renderPanel(const GuiScenario& scenario)
{
    ImGui::Begin("Simulation");
    ImGui::Text("Status: %s", stateLabel(state_));
    if (!statusMessage_.empty()) {
        if (state_ == SimulationRunState::Ready) {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f), "%s", statusMessage_.c_str());
        } else {
            ImGui::TextWrapped("%s", statusMessage_.c_str());
        }
    }
    ImGui::Text("Time: %.2f s / %.2f s", elapsedSeconds_, maxSimulationSeconds);
    ImGui::ProgressBar(progress(), ImVec2(-1.0f, 0.0f));

    ImGui::SliderFloat("Simulation speed", &simulationSpeedMultiplier_, minSimulationSpeedMultiplier, maxSimulationSpeedMultiplier, "%.3fx");
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
        recomputeOverlay();
    }
    if (ImGui::Checkbox("Show ray tracing visualization", &showRayTracing_)) {
        recomputeOverlay();
    }
    if (configuredRayCount_ > 0) {
        ImGui::Text("Configured rays: %d", configuredRayCount_);
    }
    ImGui::Text("Rays: %d total / %d active", rayCount(), activeRayCount());
    ImGui::TextWrapped("Simulation data is provided by the configured GUI simulation service.");

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
    resultOverlay_ = {};
    statusMessage_.clear();
    configuredRayCount_ = 0;
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

void SimulationScreen::recomputeOverlay()
{
    overlay_ = resultOverlay_;
    overlay_.active = state_ != SimulationRunState::Ready;
    overlay_.showDiffuseEnergy = showDiffuseEnergyOnPlanes_;
    overlay_.showRayTracing = showRayTracing_;

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
