#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

namespace gui {

class IScenarioService {
public:
    virtual ~IScenarioService() = default;

    virtual GuiScenario createInitialScenario(GuiSelection& selection) = 0;
};

}  // namespace gui
