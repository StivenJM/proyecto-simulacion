#pragma once

#include "gui/services/IPlaneService.h"
#include "gui/services/IScenarioService.h"

#include <memory>

namespace gui {

struct GuiServices {
    std::unique_ptr<IPlaneService> planeService;
    std::unique_ptr<IScenarioService> scenarioService;
};

}  // namespace gui
