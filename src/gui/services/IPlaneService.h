#pragma once

#include "gui/entities/GuiScenario.h"
#include "gui/entities/GuiSelection.h"

#include <cstddef>
#include <string>
#include <vector>

namespace gui {

class IPlaneService {
public:
    virtual ~IPlaneService() = default;

    virtual void addPlane(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void addRoom(GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points) = 0;
    virtual bool updatePlanePoint(GuiScenario& scenario, GuiSelection& selection, int planeId, std::size_t pointIndex, Vec3 point) = 0;
    virtual bool updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points) = 0;
    virtual bool updatePlaneName(GuiScenario& scenario, int planeId, const std::string& name) = 0;
    virtual bool updatePlaneAbsorption(GuiScenario& scenario, int planeId, float absorption) = 0;
    virtual bool updatePlaneVisibility(GuiScenario& scenario, int planeId, bool visible) = 0;
    virtual bool updatePlaneColor(GuiScenario& scenario, int planeId, Vec3 color) = 0;
    virtual void selectNext(const GuiScenario& scenario, GuiSelection& selection) = 0;
    virtual void moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta) = 0;
};

}  // namespace gui
