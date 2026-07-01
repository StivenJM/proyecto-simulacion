#pragma once

#include "gui/math/MathTypes.h"

#include <vector>

namespace gui {

struct SimulationStateDto {
    bool running = false;
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
    float activeEnergy = 0.0f;
    float activeRadius = 0.0f;
    bool alive = false;
    bool visible = true;
};

}  // namespace gui
