#pragma once

#include "gui/input/CameraController.h"

#include <vector>

namespace gui {

enum class PlaneOrientation {
    Horizontal,
    VerticalX,
    VerticalZ,
};

struct GuiPlane {
    int id = 0;
    Vec3 center{0.0f, 0.0f, 0.0f};
    float width = 1.0f;
    float height = 1.0f;
    PlaneOrientation orientation = PlaneOrientation::Horizontal;
    float absorption = 0.0f;
    std::vector<Vec3> outlinePoints;
};

}  // namespace gui
