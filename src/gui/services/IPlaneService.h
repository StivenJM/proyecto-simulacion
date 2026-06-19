#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

#include <vector>

namespace gui {

class IPlaneService {
public:
    virtual ~IPlaneService() = default;

    virtual void addPlane(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void addRoom(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) = 0;
    virtual bool updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) = 0;
    virtual void selectNext(const GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) = 0;
};

}  // namespace gui
