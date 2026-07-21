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
constexpr float initialRayParticleRadius = 0.08f;

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

float energyRatio(float energy, float initialEnergy)
{
    if (!std::isfinite(energy) || !std::isfinite(initialEnergy) || initialEnergy <= 0.0f) {
        return 0.0f;
    }

    return clamp01(energy / initialEnergy);
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

const char* stateLabel(SimulationRunState state)
{
    if (state == SimulationRunState::Running) return "Running";
    if (state == SimulationRunState::Finished) return "Finished";
    return "Ready";
}

float sanitizeSimulationSpeed(float value)
{
    if (!std::isfinite(value)) {
        return 1.0f;
    }

    return std::clamp(value, minSimulationSpeedMultiplier, maxSimulationSpeedMultiplier);
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

bool SimulationScreen::restart(const GuiScenario& scenario)
{
    reset();
    return start(scenario);
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

bool SimulationScreen::renderPanel(const GuiScenario& scenario)
{
    bool simulationStarted = false;

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

    if (editingSimulationSpeed_) {
        if (focusSimulationSpeedInput_) {
            ImGui::SetKeyboardFocusHere();
            focusSimulationSpeedInput_ = false;
        }

        if (ImGui::InputFloat("Simulation speed", &simulationSpeedMultiplier_, 0.001f, 0.1f, "%.3fx", ImGuiInputTextFlags_EnterReturnsTrue)) {
            simulationSpeedMultiplier_ = sanitizeSimulationSpeed(simulationSpeedMultiplier_);
            editingSimulationSpeed_ = false;
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            simulationSpeedMultiplier_ = sanitizeSimulationSpeed(simulationSpeedMultiplier_);
            editingSimulationSpeed_ = false;
        }
    } else {
        ImGuiSliderFlags speedSliderFlags = ImGuiSliderFlags_Logarithmic;
        if (ImGui::SliderFloat("Simulation speed", &simulationSpeedMultiplier_, minSimulationSpeedMultiplier, maxSimulationSpeedMultiplier, "%.3fx", speedSliderFlags)) {
            simulationSpeedMultiplier_ = sanitizeSimulationSpeed(simulationSpeedMultiplier_);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            editingSimulationSpeed_ = true;
            focusSimulationSpeedInput_ = true;
        }
    }
    if (state_ == SimulationRunState::Running) {
        if (ImGui::Button("Restart")) {
            simulationStarted = restart(scenario);
        }
    } else {
        if (ImGui::Button(state_ == SimulationRunState::Finished ? "Restart" : "Start")) {
            simulationStarted = restart(scenario);
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

    return simulationStarted;
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
        const float segmentEnergy = activeSegment->startEnergy + (activeSegment->endEnergy - activeSegment->startEnergy) * localT;
        const float relativeEnergy = energyRatio(segmentEnergy, ray.initialEnergy);
        ray.activePosition = lerp(activeSegment->start, activeSegment->end, localT);
        ray.activeEnergy = segmentEnergy;
        ray.activeRadius = initialRayParticleRadius * relativeEnergy;
        ray.alive = relativeEnergy > 0.01f;

        if (ray.alive) {
            overlay_.rayParticles.push_back({ray.activePosition, ray.activeRadius, ray.activeEnergy, ray.initialEnergy});
        }
    }
}

}  // namespace gui
