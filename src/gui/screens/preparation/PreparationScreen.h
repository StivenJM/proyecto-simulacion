#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"
#include "gui/entities/GuiPlaneDraft.h"
#include "gui/input/InputState.h"
#include "gui/services/IPlaneService.h"
#include "gui/services/IReceiverService.h"
#include "gui/services/ISourceService.h"

#include <cstddef>
#include <string>

namespace gui {

class PreparationScreen {
public:
    PreparationScreen(
        GuiScenario& scenario,
        GuiSelection& selection,
        IPlaneService& planeService,
        ISourceService& sourceService,
        IReceiverService& receiverService
    );

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
    int selectedSourceId() const;
    int selectedReceiverId() const;
    void selectPlane(int planeId);
    void selectSource(int sourceId);
    void selectReceiver(int receiverId);
    GuiScenario& scenario();
    const GuiScenario& scenario() const;
    GuiSelection& selection();
    const GuiPlane* selectedPlane() const;
    const GuiSource* selectedSource() const;
    const GuiReceiver* selectedReceiver() const;
    void addDefaultPlane();
    void addDefaultSource();
    void addDefaultReceiver();
    bool updateSelectedPlanePoint(std::size_t pointIndex, Vec3 point);
    bool updateSelectedPlaneName(const std::string& name);
    bool updateSelectedPlaneAbsorption(float absorption);
    bool updateSelectedPlaneVisibility(bool visible);
    bool updateSelectedPlaneColor(Vec3 color);
    bool updateSelectedSourcePosition(Vec3 position);
    bool updateSelectedSourceName(const std::string& name);
    bool updateSelectedSourceVisibility(bool visible);
    bool updateSelectedReceiverPosition(Vec3 position);
    bool updateSelectedReceiverName(const std::string& name);
    bool updateSelectedReceiverVisibility(bool visible);
    const GuiPlaneDraft& draft() const;

private:
    GuiScenario& scenario_;
    GuiSelection& selection_;
    IPlaneService& planeService_;
    ISourceService& sourceService_;
    IReceiverService& receiverService_;
    GuiPlaneDraft draft_;
};

}  // namespace gui
