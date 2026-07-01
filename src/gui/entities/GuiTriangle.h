#pragma once

#include "gui/input/CameraController.h"

#include <array>

namespace gui {

struct GuiTriangle {
    int id = 0;
    int planeId = 0;
    std::array<Vec3, 3> vertices{};
    Vec3 centroid{0.0f, 0.0f, 0.0f};
    float area = 0.0f;
    bool visible = true;
    float distanceToPlaneCenter = 0.0f;
};

}  // namespace gui
