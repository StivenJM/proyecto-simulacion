#pragma once

#include "gui/entities/GuiPlane.h"

#include <vector>

namespace gui {

struct PlaneDto {
    int id = 0;
    Vec3 center{0.0f, 0.0f, 0.0f};
    float width = 1.0f;
    float height = 1.0f;
    PlaneOrientation orientation = PlaneOrientation::Horizontal;
    float absorption = 0.0f;
    std::vector<Vec3> outlinePoints;
};

}  // namespace gui
