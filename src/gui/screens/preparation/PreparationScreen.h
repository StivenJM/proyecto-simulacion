#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"
#include "gui/entities/GuiPlaneDraft.h"
#include "gui/input/InputState.h"
#include "gui/services/IPlaneService.h"

#include <cstddef>
#include <string>

namespace gui {

class PreparationScreen {
public:
    PreparationScreen(GuiScenario& scenario, GuiSelection& selection, IPlaneService& planeService);

    bool handleInput(const InputState& input, float deltaTime);
    void startDraft();
    void addDraftPoint();
    bool finalizeDraft();
    bool editSelectedPlane();
    void cancelDraft();
    void setDraftCursor(Vec3 cursor);
    Vec3 draftCursor() const;
    std::size_t draftPointCount() const;
    bool canFinalizeDraft() const;
    bool hasActiveDraft() const;
    int selectedPlaneId() const;
    void selectPlane(int planeId);
    GuiScenario& scenario();
    const GuiScenario& scenario() const;
    GuiSelection& selection();
    const GuiPlane* selectedPlane() const;
    void addDefaultPlane();
    bool updateSelectedPlanePoint(std::size_t pointIndex, Vec3 point);
    bool updateSelectedPlaneName(const std::string& name);
    bool updateSelectedPlaneAbsorption(float absorption);
    bool updateSelectedPlaneVisibility(bool visible);
    bool updateSelectedPlaneColor(Vec3 color);
    bool updateSelectedPlaneMaterial(int materialId);
    const GuiPlaneDraft& draft() const;

private:
    GuiScenario& scenario_;
    GuiSelection& selection_;
    IPlaneService& planeService_;
    GuiPlaneDraft draft_;
};

}  // namespace gui
