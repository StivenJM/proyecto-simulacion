#pragma once

#include "gui/services/IPlaneService.h"
#include "gui/services/IReceiverService.h"
#include "gui/services/IScenarioService.h"
#include "gui/services/ISourceService.h"

#include <memory>

namespace gui {

struct GuiServices {
    std::unique_ptr<IPlaneService> planeService;
    std::unique_ptr<ISourceService> sourceService;
    std::unique_ptr<IReceiverService> receiverService;
    std::unique_ptr<IScenarioService> scenarioService;
};

}  // namespace gui
