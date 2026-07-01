#pragma once

#include "GuiServices.h"
#include "gui/config/GuiConfig.h"

namespace gui {

class GuiComposition {
public:
    static GuiServices createServices(const GuiConfig& config);
};

}  // namespace gui
