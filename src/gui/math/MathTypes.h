#pragma once

#include <array>

namespace gui {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Mat4 {
    std::array<float, 16> values{};
};

}  // namespace gui
