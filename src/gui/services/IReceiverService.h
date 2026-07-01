#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

#include <string>

namespace gui {

class IReceiverService {
public:
    virtual ~IReceiverService() = default;

    virtual void addReceiver(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual bool updateReceiverPosition(GuiScenario& scenario, int receiverId, Vec3 position) = 0;
    virtual bool updateReceiverName(GuiScenario& scenario, int receiverId, const std::string& name) = 0;
    virtual bool updateReceiverVisibility(GuiScenario& scenario, int receiverId, bool visible) = 0;
    virtual void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) = 0;
};

}  // namespace gui
