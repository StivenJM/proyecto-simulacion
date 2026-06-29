#pragma once

#include "gui/input/CameraController.h"

#include <vector>

namespace gui {

struct LineVertex {
    Vec3 position;
    Vec3 color;
};

struct ColorRgba {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct ColoredVertex {
    Vec3 position;
    ColorRgba color;
};

struct RenderScene {
    std::vector<ColoredVertex> opaqueFillVertices;
    std::vector<ColoredVertex> fillVertices;
    std::vector<LineVertex> lineVertices;
};

}  // namespace gui
