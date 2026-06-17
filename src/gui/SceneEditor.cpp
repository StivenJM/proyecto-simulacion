#include "SceneEditor.h"

namespace gui {
namespace {

void addLine(std::vector<LineVertex>& vertices, Vec3 from, Vec3 to, Vec3 color)
{
    vertices.push_back({from, color});
    vertices.push_back({to, color});
}

void appendPlaneLines(std::vector<LineVertex>& vertices, const EditablePlane& plane, bool selected)
{
    const float halfWidth = plane.width / 2.0f;
    const float halfHeight = plane.height / 2.0f;
    const Vec3 color = selected ? Vec3{1.0f, 0.86f, 0.18f} : Vec3{0.72f, 0.78f, 0.86f};

    Vec3 a{};
    Vec3 b{};
    Vec3 c{};
    Vec3 d{};

    if (plane.orientation == PlaneOrientation::Horizontal) {
        a = {plane.center.x - halfWidth, plane.center.y, plane.center.z - halfHeight};
        b = {plane.center.x + halfWidth, plane.center.y, plane.center.z - halfHeight};
        c = {plane.center.x + halfWidth, plane.center.y, plane.center.z + halfHeight};
        d = {plane.center.x - halfWidth, plane.center.y, plane.center.z + halfHeight};
    } else if (plane.orientation == PlaneOrientation::VerticalX) {
        a = {plane.center.x, plane.center.y - halfHeight, plane.center.z - halfWidth};
        b = {plane.center.x, plane.center.y - halfHeight, plane.center.z + halfWidth};
        c = {plane.center.x, plane.center.y + halfHeight, plane.center.z + halfWidth};
        d = {plane.center.x, plane.center.y + halfHeight, plane.center.z - halfWidth};
    } else {
        a = {plane.center.x - halfWidth, plane.center.y - halfHeight, plane.center.z};
        b = {plane.center.x + halfWidth, plane.center.y - halfHeight, plane.center.z};
        c = {plane.center.x + halfWidth, plane.center.y + halfHeight, plane.center.z};
        d = {plane.center.x - halfWidth, plane.center.y + halfHeight, plane.center.z};
    }

    addLine(vertices, a, b, color);
    addLine(vertices, b, c, color);
    addLine(vertices, c, d, color);
    addLine(vertices, d, a, color);

    if (selected) {
        addLine(vertices, a, c, color);
        addLine(vertices, b, d, color);
    }
}

void appendAxes(std::vector<LineVertex>& vertices)
{
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.25f, 0.25f});
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.25f, 1.0f, 0.25f});
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.25f, 0.45f, 1.0f});
}

}  // namespace

SceneEditor::SceneEditor()
{
    addRoom();
}

void SceneEditor::addPlane()
{
    const float offset = static_cast<float>(planes_.size()) * 0.25f;
    addPlane({offset, -1.0f, offset}, 2.0f, 1.5f, PlaneOrientation::Horizontal);
}

void SceneEditor::addRoom()
{
    addPlane({0.0f, -1.0f, 0.0f}, 4.0f, 3.0f, PlaneOrientation::Horizontal);
    addPlane({0.0f, 1.0f, 0.0f}, 4.0f, 3.0f, PlaneOrientation::Horizontal);
    addPlane({-2.0f, 0.0f, 0.0f}, 3.0f, 2.0f, PlaneOrientation::VerticalX);
    addPlane({2.0f, 0.0f, 0.0f}, 3.0f, 2.0f, PlaneOrientation::VerticalX);
    addPlane({0.0f, 0.0f, -1.5f}, 4.0f, 2.0f, PlaneOrientation::VerticalZ);
    addPlane({0.0f, 0.0f, 1.5f}, 4.0f, 2.0f, PlaneOrientation::VerticalZ);
}

void SceneEditor::selectNext()
{
    if (planes_.empty()) {
        selectedPlaneId_ = 0;
        return;
    }

    for (std::size_t index = 0; index < planes_.size(); ++index) {
        if (planes_[index].id == selectedPlaneId_) {
            const std::size_t nextIndex = (index + 1) % planes_.size();
            selectedPlaneId_ = planes_[nextIndex].id;
            return;
        }
    }

    selectedPlaneId_ = planes_.front().id;
}

void SceneEditor::moveSelected(Vec3 delta)
{
    for (EditablePlane& plane : planes_) {
        if (plane.id == selectedPlaneId_) {
            plane.center.x += delta.x;
            plane.center.y += delta.y;
            plane.center.z += delta.z;
            return;
        }
    }
}

int SceneEditor::selectedPlaneId() const
{
    return selectedPlaneId_;
}

const std::vector<EditablePlane>& SceneEditor::planes() const
{
    return planes_;
}

std::vector<LineVertex> SceneEditor::buildLineVertices() const
{
    std::vector<LineVertex> vertices;
    vertices.reserve(planes_.size() * 12 + 6);

    for (const EditablePlane& plane : planes_) {
        appendPlaneLines(vertices, plane, plane.id == selectedPlaneId_);
    }

    appendAxes(vertices);
    return vertices;
}

void SceneEditor::addPlane(Vec3 center, float width, float height, PlaneOrientation orientation)
{
    const int id = nextId_++;
    planes_.push_back({id, center, width, height, orientation});
    selectedPlaneId_ = id;
}

}  // namespace gui
