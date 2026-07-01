#pragma once

#include "gui/input/CameraController.h"

#include <string>

namespace gui {

struct GuiSource {
    int id = 0;
    std::string name;
    Vec3 position{0.0f, 0.0f, 0.0f};
    bool visible = true;
    Vec3 color{1.0f, 132.0f / 255.0f, 120.0f / 255.0f};
};

}  // namespace gui
