#pragma once

#include "GuiPlane.h"
#include "GuiReceiver.h"
#include "GuiSimulationConfig.h"
#include "GuiSource.h"

#include <vector>

namespace gui {

struct GuiScenario {
    GuiSimulationConfig simulationConfig;
    int nextPlaneId = 1;
    int nextSourceId = 1;
    int nextReceiverId = 1;
    std::vector<GuiPlane> planes;
    std::vector<GuiSource> sources;
    std::vector<GuiReceiver> receivers;
};

}  // namespace gui
