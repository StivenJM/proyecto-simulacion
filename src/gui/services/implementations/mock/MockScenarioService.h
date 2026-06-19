#pragma once

#include "MockPlaneService.h"
#include "gui/services/IScenarioService.h"

namespace gui {

class MockScenarioService final : public IScenarioService {
public:
    GuiScenario createInitialScenario(GuiSelection& selection) override;

private:
    MockPlaneService planeService_;
};

}  // namespace gui
