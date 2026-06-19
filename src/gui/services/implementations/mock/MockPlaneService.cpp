#include "MockPlaneService.h"

namespace gui {
namespace {

Vec3 averagePoint(const std::vector<Vec3>& points)
{
    Vec3 center{0.0f, 0.0f, 0.0f};
    if (points.empty()) {
        return center;
    }

    for (const Vec3& point : points) {
        center.x += point.x;
        center.y += point.y;
        center.z += point.z;
    }

    const float count = static_cast<float>(points.size());
    return {center.x / count, center.y / count, center.z / count};
}

}  // namespace

void MockPlaneService::addPlane(GuiScenario& scenario, GuiSelection& selection)
{
    const float offset = static_cast<float>(scenario.planes.size()) * 0.25f;
    addPlane(scenario, selection, {offset, -1.0f, offset}, 2.0f, 1.5f, PlaneOrientation::Horizontal);
}

void MockPlaneService::addRoom(GuiScenario& scenario, GuiSelection& selection)
{
    addPlane(scenario, selection, {0.0f, -1.0f, 0.0f}, 4.0f, 3.0f, PlaneOrientation::Horizontal);
    addPlane(scenario, selection, {0.0f, 1.0f, 0.0f}, 4.0f, 3.0f, PlaneOrientation::Horizontal);
    addPlane(scenario, selection, {-2.0f, 0.0f, 0.0f}, 3.0f, 2.0f, PlaneOrientation::VerticalX);
    addPlane(scenario, selection, {2.0f, 0.0f, 0.0f}, 3.0f, 2.0f, PlaneOrientation::VerticalX);
    addPlane(scenario, selection, {0.0f, 0.0f, -1.5f}, 4.0f, 2.0f, PlaneOrientation::VerticalZ);
    addPlane(scenario, selection, {0.0f, 0.0f, 1.5f}, 4.0f, 2.0f, PlaneOrientation::VerticalZ);
}

void MockPlaneService::addPlaneFromPoints(GuiScenario& scenario, GuiSelection& selection, const std::vector<Vec3>& points)
{
    if (points.size() < 3) {
        return;
    }

    const int id = scenario.nextPlaneId++;
    scenario.planes.push_back({id, averagePoint(points), 1.0f, 1.0f, PlaneOrientation::Horizontal, 0.0f, points});
    selection.selectedPlaneId = id;
}

bool MockPlaneService::updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points)
{
    if (points.size() < 3) {
        return false;
    }

    for (GuiPlane& plane : scenario.planes) {
        if (plane.id == planeId) {
            plane.outlinePoints = points;
            plane.center = averagePoint(points);
            selection.selectedPlaneId = planeId;
            return true;
        }
    }

    return false;
}

void MockPlaneService::selectNext(const GuiScenario& scenario, GuiSelection& selection)
{
    if (scenario.planes.empty()) {
        selection.selectedPlaneId = 0;
        return;
    }

    for (std::size_t index = 0; index < scenario.planes.size(); ++index) {
        if (scenario.planes[index].id == selection.selectedPlaneId) {
            const std::size_t nextIndex = (index + 1) % scenario.planes.size();
            selection.selectedPlaneId = scenario.planes[nextIndex].id;
            return;
        }
    }

    selection.selectedPlaneId = scenario.planes.front().id;
}

void MockPlaneService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta)
{
    for (GuiPlane& plane : scenario.planes) {
        if (plane.id == selection.selectedPlaneId) {
            plane.center.x += delta.x;
            plane.center.y += delta.y;
            plane.center.z += delta.z;
            for (Vec3& point : plane.outlinePoints) {
                point.x += delta.x;
                point.y += delta.y;
                point.z += delta.z;
            }
            return;
        }
    }
}

void MockPlaneService::addPlane(
    GuiScenario& scenario,
    GuiSelection& selection,
    Vec3 center,
    float width,
    float height,
    PlaneOrientation orientation
)
{
    const int id = scenario.nextPlaneId++;
    scenario.planes.push_back({id, center, width, height, orientation, 0.0f, {}});
    selection.selectedPlaneId = id;
}

}  // namespace gui
