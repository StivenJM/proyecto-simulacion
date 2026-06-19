#pragma once

#include "gui/entities/GuiPlaneDraft.h"
#include "gui/rendering/RenderTypes.h"

#include <vector>

namespace gui {

class PreparationSidePanel {
public:
    static void appendLines(std::vector<LineVertex>& vertices, const GuiPlaneDraft& draft, int selectedPlaneId);
};

}  // namespace gui
