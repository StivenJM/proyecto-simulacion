#pragma once

#include "gui/input/CameraController.h"

#include <vector>

namespace gui {

struct GuiPlaneDraft {
    bool active = false;
    int editingPlaneId = 0;
    Vec3 cursor{0.0f, 0.0f, 0.0f};
    std::vector<Vec3> points;

    bool isEditingExistingPlane() const { return editingPlaneId != 0; }
    bool canFinalize() const { return points.size() >= 3; }
};

}  // namespace gui
