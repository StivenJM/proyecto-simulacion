#pragma once

#include "gui/rendering/RenderTypes.h"

#include <vector>

namespace gui {

class AxisGizmo {
public:
    static void appendLines(std::vector<LineVertex>& vertices);
};

}  // namespace gui
