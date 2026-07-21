#pragma once

#include "RenderTypes.h"
#include "gui/entities/GuiPlaneDraft.h"
#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

#include <vector>

namespace gui {

class GuiScenarioRenderMapper {
public:
    RenderScene buildRenderScene(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft) const;
    RenderScene buildRenderScene(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft, const RenderSimulationOverlay& simulationOverlay) const;
    RenderScene buildRenderScene(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft, const RenderSimulationOverlay& simulationOverlay, Vec3 cameraPosition) const;
    std::vector<LineVertex> buildLineVertices(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft) const;
};

}  // namespace gui
