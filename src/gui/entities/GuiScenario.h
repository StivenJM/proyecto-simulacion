#pragma once

#include "GuiPlane.h"
#include "GuiReceiver.h"
#include "GuiSource.h"

#include <vector>

namespace gui {

struct GuiScenario {
    int nextPlaneId = 1;
    int nextSourceId = 1;
    int nextReceiverId = 1;
    std::vector<GuiPlane> planes;
    std::vector<GuiSource> sources;
    std::vector<GuiReceiver> receivers;
};

}  // namespace gui
