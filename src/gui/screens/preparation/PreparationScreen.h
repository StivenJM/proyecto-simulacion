#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"
#include "gui/entities/GuiPlaneDraft.h"
#include "gui/input/InputState.h"
#include "gui/services/IPlaneService.h"

namespace gui {

class PreparationScreen {
public:
    PreparationScreen(GuiScenario& scenario, GuiSelection& selection, IPlaneService& planeService);

    bool handleInput(const InputState& input, float deltaTime);
    int selectedPlaneId() const;
    const GuiPlaneDraft& draft() const;

private:
    GuiScenario& scenario_;
    GuiSelection& selection_;
    IPlaneService& planeService_;
    GuiPlaneDraft draft_;
};

}  // namespace gui
