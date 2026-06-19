#pragma once

#include "gui/services/implementations/mock/MockScenarioService.h"

namespace gui {

class CoreScenarioService final : public IScenarioService {
public:
    GuiScenario createInitialScenario(GuiSelection& selection) override;

private:
    MockScenarioService fallback_;
};

}  // namespace gui
