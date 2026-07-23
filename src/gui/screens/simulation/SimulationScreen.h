#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/rendering/RenderTypes.h"
#include "gui/services/ISimulationService.h"
#include "gui/services/dtos/SimulationDtos.h"

#include <chrono>
#include <cstdint>
#include <future>
#include <string>
#include <vector>

namespace gui {

enum class SimulationRunState {
    Ready,
    Running,
    Finished,
};

enum class SimulationViewMode {
    External,
    Internal,
};

enum class SimulationPrecomputeState {
    Idle,
    Computing,
    Ready,
    Failed,
};

struct SimulatedPlaneEnergy {
    int planeId = 0;
    std::string planeName;
    float energy = 0.0f;
};

class SimulationScreen {
public:
    explicit SimulationScreen(ISimulationService& simulationService);

    void beginPrecompute(const GuiScenario& scenario);
    bool start(const GuiScenario& scenario);
    bool restart(const GuiScenario& scenario);
    void update(float deltaTime, const GuiScenario& scenario);
    bool renderPanel(const GuiScenario& scenario);
    void reset();
    bool isPrecomputeReady() const;
    bool isStarted() const;
    bool isRunning() const;
    bool isFinished() const;
    float elapsedSeconds() const;
    float progress() const;
    SimulationViewMode viewMode() const;
    RenderSimulationOverlay renderOverlay() const;
    const std::vector<SimulatedPlaneEnergy>& planeEnergy() const;
    int rayCount() const;
    int activeRayCount() const;

private:
    void launchPrecompute(GuiScenario scenario, std::uint64_t generation);
    void pollPrecompute();
    void applySimulationResult(const SimulationResultDto& result);
    void recomputeOverlay();
    void recomputeTriangleEnergy(float progress);
    void recomputeRays(float progress);

    ISimulationService& simulationService_;
    SimulationRunState state_ = SimulationRunState::Ready;
    SimulationPrecomputeState precomputeState_ = SimulationPrecomputeState::Idle;
    SimulationViewMode viewMode_ = SimulationViewMode::External;
    float elapsedSeconds_ = 0.0f;
    float precomputeElapsedSeconds_ = 0.0f;
    float simulationSpeedMultiplier_ = 1.0f;
    bool editingSimulationSpeed_ = false;
    bool focusSimulationSpeedInput_ = false;
    bool showDiffuseEnergyOnPlanes_ = true;
    bool showRayTracing_ = true;
    std::vector<SimulatedPlaneEnergy> planeEnergy_;
    std::vector<GuiSimulationRay> rays_;
    RenderSimulationOverlay overlay_;
    RenderSimulationOverlay resultOverlay_;
    std::string statusMessage_;
    int configuredRayCount_ = 0;
    std::future<SimulationResultDto> precomputeFuture_;
    SimulationResultDto precomputedResult_;
    GuiScenario pendingPrecomputeScenario_;
    std::chrono::steady_clock::time_point precomputeStartTime_{};
    std::uint64_t precomputeGeneration_ = 0;
    std::uint64_t futureGeneration_ = 0;
    bool precomputeLaunchPending_ = false;
};

}  // namespace gui
