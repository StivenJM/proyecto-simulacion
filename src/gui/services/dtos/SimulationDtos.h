#pragma once

#include "gui/math/MathTypes.h"
#include "gui/rendering/RenderTypes.h"

#include <string>
#include <vector>

namespace gui {

struct SimulationStateDto {
    bool running = false;
};

struct SimulationPlaneEnergyDto {
    int planeId = 0;
    std::string planeName;
    float energy = 0.0f;
};

struct GuiRaySegment {
    Vec3 start{0.0f, 0.0f, 0.0f};
    Vec3 end{0.0f, 0.0f, 0.0f};
    int planeId = -1;
    int triangleId = -1;
    float startTimeSeconds = 0.0f;
    float endTimeSeconds = 0.0f;
    float startEnergy = 1.0f;
    float endEnergy = 1.0f;
};

struct GuiSimulationRay {
    int id = 0;
    int sourceId = 0;
    std::vector<GuiRaySegment> segments;
    Vec3 activePosition{0.0f, 0.0f, 0.0f};
    float initialEnergy = 1.0f;
    float activeEnergy = 0.0f;
    float activeRadius = 0.0f;
    bool alive = false;
    bool visible = true;
};

struct DiffusionTriangleDto {
    int triangleId = 0;
    int planeId = 0;
};

struct DiffusionDataDto {
    std::vector<DiffusionTriangleDto> triangles;
    std::vector<std::vector<double>> distances;
    std::vector<std::vector<int>> timesMs;
    std::vector<std::vector<double>> percentages;
    std::vector<std::vector<bool>> visibility;
    std::vector<std::vector<double>> energyByTriangleTime;
    int timeStepMs = 1;
};

struct SimulationResultDto {
    SimulationStateDto state;
    bool success = false;
    std::string message;
    float durationSeconds = 1.0f;
    int configuredRayCount = 0;
    std::vector<SimulationPlaneEnergyDto> planeEnergy;
    std::vector<GuiSimulationRay> rays;
    DiffusionDataDto diffusion;
    RenderSimulationOverlay overlay;
};

}  // namespace gui
