#include "MockPlaneService.h"

#include <cmath>
#include <string>

namespace gui {
namespace {

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

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

Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

float length(Vec3 value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

Vec3 cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float triangleArea(Vec3 a, Vec3 b, Vec3 c)
{
    return length(cross(subtract(b, a), subtract(c, a))) * 0.5f;
}

float polygonArea(const std::vector<Vec3>& points)
{
    if (points.size() < 3) {
        return 0.0f;
    }

    Vec3 sum{0.0f, 0.0f, 0.0f};
    for (std::size_t index = 0; index < points.size(); ++index) {
        const Vec3& current = points[index];
        const Vec3& next = points[(index + 1) % points.size()];
        const Vec3 edgeCross = cross(current, next);
        sum.x += edgeCross.x;
        sum.y += edgeCross.y;
        sum.z += edgeCross.z;
    }

    return length(sum) * 0.5f;
}

float planeArea(const GuiPlane& plane)
{
    if (plane.outlinePoints.size() >= 3) {
        return polygonArea(plane.outlinePoints);
    }

    return plane.width * plane.height;
}

std::vector<Vec3> rectanglePoints(Vec3 center, float width, float height, PlaneOrientation orientation)
{
    const float halfWidth = width / 2.0f;
    const float halfHeight = height / 2.0f;

    if (orientation == PlaneOrientation::Horizontal) {
        return {
            {center.x - halfWidth, center.y, center.z - halfHeight},
            {center.x + halfWidth, center.y, center.z - halfHeight},
            {center.x + halfWidth, center.y, center.z + halfHeight},
            {center.x - halfWidth, center.y, center.z + halfHeight},
        };
    }

    if (orientation == PlaneOrientation::VerticalX) {
        return {
            {center.x, center.y - halfHeight, center.z - halfWidth},
            {center.x, center.y - halfHeight, center.z + halfWidth},
            {center.x, center.y + halfHeight, center.z + halfWidth},
            {center.x, center.y + halfHeight, center.z - halfWidth},
        };
    }

    return {
        {center.x - halfWidth, center.y - halfHeight, center.z},
        {center.x + halfWidth, center.y - halfHeight, center.z},
        {center.x + halfWidth, center.y + halfHeight, center.z},
        {center.x - halfWidth, center.y + halfHeight, center.z},
    };
}

void refreshDerivedPlaneData(GuiPlane& plane)
{
    if (plane.outlinePoints.size() >= 3) {
        plane.center = averagePoint(plane.outlinePoints);
    }
    plane.area = planeArea(plane);

    plane.triangles.clear();
    if (plane.outlinePoints.size() < 3) {
        return;
    }

    const Vec3 origin = plane.outlinePoints.front();
    for (std::size_t index = 1; index + 1 < plane.outlinePoints.size(); ++index) {
        const Vec3 b = plane.outlinePoints[index];
        const Vec3 c = plane.outlinePoints[index + 1];
        GuiTriangle triangle;
        triangle.id = plane.id * 1000 + static_cast<int>(index);
        triangle.planeId = plane.id;
        triangle.vertices = {origin, b, c};
        triangle.centroid = {
            (origin.x + b.x + c.x) / 3.0f,
            (origin.y + b.y + c.y) / 3.0f,
            (origin.z + b.z + c.z) / 3.0f,
        };
        triangle.area = triangleArea(origin, b, c);
        triangle.visible = true;
        triangle.distanceToPlaneCenter = length(subtract(triangle.centroid, plane.center));
        plane.triangles.push_back(triangle);
    }
}

GuiPlane* findPlane(GuiScenario& scenario, int planeId)
{
    for (GuiPlane& plane : scenario.planes) {
        if (plane.id == planeId) {
            return &plane;
        }
    }

    return nullptr;
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
    GuiPlane plane;
    plane.id = id;
    plane.center = averagePoint(points);
    plane.outlinePoints = points;
    plane.name = "Plane " + std::to_string(id);
    refreshDerivedPlaneData(plane);
    scenario.planes.push_back(plane);
    selection.selectPlane(id);
}

bool MockPlaneService::updatePlanePoint(GuiScenario& scenario, GuiSelection& selection, int planeId, std::size_t pointIndex, Vec3 point)
{
    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        if (plane->outlinePoints.size() < 3 || pointIndex >= plane->outlinePoints.size()) {
            return false;
        }

        plane->outlinePoints[pointIndex] = point;
        refreshDerivedPlaneData(*plane);
        selection.selectPlane(planeId);
        return true;
    }

    return false;
}

bool MockPlaneService::updatePlanePoints(GuiScenario& scenario, GuiSelection& selection, int planeId, const std::vector<Vec3>& points)
{
    if (points.size() < 3) {
        return false;
    }

    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        plane->outlinePoints = points;
        refreshDerivedPlaneData(*plane);
        selection.selectPlane(planeId);
        return true;
    }

    return false;
}

bool MockPlaneService::updatePlaneName(GuiScenario& scenario, int planeId, const std::string& name)
{
    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        plane->name = name;
        return true;
    }

    return false;
}

bool MockPlaneService::updatePlaneAbsorption(GuiScenario& scenario, int planeId, float absorption)
{
    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        plane->absorption = clamp01(absorption);
        return true;
    }

    return false;
}

bool MockPlaneService::updatePlaneVisibility(GuiScenario& scenario, int planeId, bool visible)
{
    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        plane->visible = visible;
        return true;
    }

    return false;
}

bool MockPlaneService::updatePlaneColor(GuiScenario& scenario, int planeId, Vec3 color)
{
    if (GuiPlane* plane = findPlane(scenario, planeId)) {
        plane->color = {clamp01(color.x), clamp01(color.y), clamp01(color.z)};
        return true;
    }

    return false;
}

void MockPlaneService::selectNext(const GuiScenario& scenario, GuiSelection& selection)
{
    if (scenario.planes.empty()) {
        selection.clear();
        return;
    }

    for (std::size_t index = 0; index < scenario.planes.size(); ++index) {
        if (scenario.planes[index].id == selection.selectedPlaneId()) {
            const std::size_t nextIndex = (index + 1) % scenario.planes.size();
            selection.selectPlane(scenario.planes[nextIndex].id);
            return;
        }
    }

    selection.selectPlane(scenario.planes.front().id);
}

void MockPlaneService::moveSelected(GuiScenario& scenario, const GuiSelection& selection, Vec3 delta)
{
    for (GuiPlane& plane : scenario.planes) {
        if (plane.id == selection.selectedPlaneId()) {
            plane.center.x += delta.x;
            plane.center.y += delta.y;
            plane.center.z += delta.z;
            for (Vec3& point : plane.outlinePoints) {
                point.x += delta.x;
                point.y += delta.y;
                point.z += delta.z;
            }
            refreshDerivedPlaneData(plane);
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
    GuiPlane plane;
    plane.id = id;
    plane.center = center;
    plane.width = width;
    plane.height = height;
    plane.orientation = orientation;
    plane.name = "Plane " + std::to_string(id);
    plane.visible = true;
    plane.absorption = 0.0f;
    plane.outlinePoints = rectanglePoints(center, width, height, orientation);
    refreshDerivedPlaneData(plane);
    scenario.planes.push_back(plane);
    selection.selectPlane(id);
}

}  // namespace gui
