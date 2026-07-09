#pragma once

#include "gui/services/implementations/mock/MockSourceService.h"

namespace gui {

class CoreSourceService final : public ISourceService {
public:
    void addSource(GuiScenario& scenario, GuiSelection& selection) override;
    bool updateSourcePosition(GuiScenario& scenario, int sourceId, Vec3 position) override;
    bool updateSourceName(GuiScenario& scenario, int sourceId, const std::string& name) override;
    bool updateSourceEnergy(GuiScenario& scenario, int sourceId, float energy) override;
    bool updateSourceVisibility(GuiScenario& scenario, int sourceId, bool visible) override;
    void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) override;

private:
    MockSourceService fallback_;
};

}  // namespace gui
