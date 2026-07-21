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
    bool isGeneralSelected() const;
    int selectedPlaneId() const;
    int selectedSourceId() const;
    int selectedReceiverId() const;
    int selectedTriangleId() const;
    void selectGeneral();
    void selectPlane(int planeId);
    void selectSource(int sourceId);
    void selectReceiver(int receiverId);
    GuiScenario& scenario();
    const GuiScenario& scenario() const;
    GuiSelection& selection();
    const GuiPlane* selectedPlane() const;
    const GuiTriangle* selectedTriangle() const;
    const GuiSource* selectedSource() const;
    const GuiReceiver* selectedReceiver() const;
    void addDefaultPlane();
    void addDefaultSource();
    void addDefaultReceiver();
    bool updateSelectedPlanePoint(std::size_t pointIndex, Vec3 point);
    bool updateSelectedPlaneName(const std::string& name);
    bool updateSelectedPlaneAbsorption(float absorption);
    void updateAllPlaneAbsorption(float absorption);
    bool updateSelectedPlaneVisibility(bool visible);
    bool updateSelectedPlaneColor(Vec3 color);
    bool updateSelectedPlaneNormal(Vec3 normal);
    void selectTriangle(int triangleId);
    bool updateSelectedSourcePosition(Vec3 position);
    bool updateSelectedSourceName(const std::string& name);
    bool updateSelectedSourceEnergy(float energy);
    bool updateSelectedSourceVisibility(bool visible);
    bool updateSelectedReceiverPosition(Vec3 position);
    bool updateSelectedReceiverName(const std::string& name);
    bool updateSelectedReceiverVisibility(bool visible);
    int rayCount() const;
    void updateRayCount(int rayCount);
    int meshSubdivisions() const;
    void updateMeshSubdivisions(int meshSubdivisions);
    float globalAbsorption() const;
    float diffusionCoefficient() const;
    void updateDiffusionCoefficient(float diffusionCoefficient);
    bool solidSceneFill() const;
    void updateSolidSceneFill(bool solidSceneFill);
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
