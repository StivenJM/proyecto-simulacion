#pragma once

#include "GuiTriangle.h"
#include "gui/input/CameraController.h"
#include "gui/math/HeatMapColor.h"

#include <string>
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
    Vec3 normal{0.0f, 1.0f, 0.0f};
    float absorption = 0.2f;
    std::vector<Vec3> outlinePoints;
    std::string name;
    bool visible = true;
    Vec3 color{heatMapColorVec3(0.0)};
    float area = 1.0f;
    std::vector<GuiTriangle> triangles;
};

}  // namespace gui
