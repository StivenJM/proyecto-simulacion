#pragma once

#include "gui/services/implementations/mock/MockReceiverService.h"

namespace gui {

class CoreReceiverService final : public IReceiverService {
public:
    void addReceiver(GuiScenario& scenario, GuiSelection& selection) override;
    bool updateReceiverPosition(GuiScenario& scenario, int receiverId, Vec3 position) override;
    bool updateReceiverName(GuiScenario& scenario, int receiverId, const std::string& name) override;
    bool updateReceiverVisibility(GuiScenario& scenario, int receiverId, bool visible) override;
    void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) override;

private:
    MockReceiverService fallback_;
};

}  // namespace gui
