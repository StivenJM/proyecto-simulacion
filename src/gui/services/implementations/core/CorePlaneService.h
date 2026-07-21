#pragma once

#include "gui/services/implementations/mock/MockPlaneService.h"

namespace gui {

class CorePlaneService final : public IPlaneService {
public:
    void addPlane(GuiScenario& scenario, GuiSelection& selection) override;
    void addRoom(GuiScenario& scenario, GuiSelection& selection) override;
    void addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) override;
    bool updatePlanePoint(GuiScenario& scenario, GuiSelection& selection, int planeId, std::size_t pointIndex, Vec3 point) override;
    bool updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) override;
    bool updatePlaneName(GuiScenario& scenario, int planeId, const std::string& name) override;
    bool updatePlaneAbsorption(GuiScenario& scenario, int planeId, float absorption) override;
    void updateAllPlaneAbsorption(GuiScenario& scenario, float absorption) override;
    bool updatePlaneVisibility(GuiScenario& scenario, int planeId, bool visible) override;
    bool updatePlaneColor(GuiScenario& scenario, int planeId, Vec3 color) override;
    bool updatePlaneNormal(GuiScenario& scenario, int planeId, Vec3 normal) override;
    void selectNext(const GuiScenario& scenario, GuiSelection& selection) override;
    void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) override;

private:
    MockPlaneService fallback_;
};

}  // namespace gui
