#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

#include <string>

namespace gui {

class ISourceService {
public:
    virtual ~ISourceService() = default;

    virtual void addSource(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual bool updateSourcePosition(GuiScenario& scenario, int sourceId, Vec3 position) = 0;
    virtual bool updateSourceName(GuiScenario& scenario, int sourceId, const std::string& name) = 0;
    virtual bool updateSourceEnergy(GuiScenario& scenario, int sourceId, float energy) = 0;
    virtual bool updateSourceVisibility(GuiScenario& scenario, int sourceId, bool visible) = 0;
    virtual void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) = 0;
};

}  // namespace gui
