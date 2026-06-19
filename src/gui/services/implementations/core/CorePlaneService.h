#pragma once

#include "gui/services/implementations/mock/MockPlaneService.h"

namespace gui {

class CorePlaneService final : public IPlaneService {
public:
    void addPlane(GuiScenario& scenario, GuiSelection& selection) override;
    void addRoom(GuiScenario& scenario, GuiSelection& selection) override;
    void addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) override;
    bool updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) override;
    void selectNext(const GuiScenario& scenario, GuiSelection& selection) override;
    void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) override;

private:
    MockPlaneService fallback_;
};

}  // namespace gui
