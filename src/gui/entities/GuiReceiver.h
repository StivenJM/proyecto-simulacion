#pragma once

#include "gui/input/CameraController.h"

#include <string>

namespace gui {

struct GuiReceiver {
    int id = 0;
    std::string name;
    Vec3 position{0.0f, 0.0f, 0.0f};
    bool visible = true;
    Vec3 color{125.0f / 255.0f, 251.0f / 255.0f, 126.0f / 255.0f};
};

}  // namespace gui
