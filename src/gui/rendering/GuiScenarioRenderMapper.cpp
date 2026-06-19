#include "GuiScenarioRenderMapper.h"

#include "gui/screens/components/AxisGizmo.h"
#include "gui/screens/preparation/components/PreparationSidePanel.h"

namespace gui {
namespace {

void addLine(std::vector<LineVertex>& vertices, Vec3 from, Vec3 to, Vec3 color)
{
    vertices.push_back({from, color});
    vertices.push_back({to, color});
}

void appendPlaneLines(std::vector<LineVertex>& vertices, const GuiPlane& plane, bool selected)
{
    const Vec3 color = selected ? Vec3{1.0f, 0.86f, 0.18f} : Vec3{0.72f, 0.78f, 0.86f};

    if (plane.outlinePoints.size() >= 3) {
        for (std::size_t index = 0; index < plane.outlinePoints.size(); ++index) {
            const Vec3 from = plane.outlinePoints[index];
            const Vec3 to = plane.outlinePoints[(index + 1) % plane.outlinePoints.size()];
            addLine(vertices, from, to, color);
        }

        if (selected) {
            addLine(vertices, plane.center, plane.outlinePoints.front(), color);
        }

        return;
    }

    const float halfWidth = plane.width / 2.0f;
    const float halfHeight = plane.height / 2.0f;

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

void appendPointMarker(std::vector<LineVertex>& vertices, Vec3 point, Vec3 color)
{
    constexpr float size = 0.06f;
    addLine(vertices, {point.x - size, point.y, point.z}, {point.x + size, point.y, point.z}, color);
    addLine(vertices, {point.x, point.y - size, point.z}, {point.x, point.y + size, point.z}, color);
    addLine(vertices, {point.x, point.y, point.z - size}, {point.x, point.y, point.z + size}, color);
}

void appendDraftLines(std::vector<LineVertex>& vertices, const GuiPlaneDraft& draft)
{
    if (!draft.active) {
        return;
    }

    const Vec3 lineColor = draft.isEditingExistingPlane() ? Vec3{0.65f, 0.42f, 1.0f} : Vec3{0.25f, 0.95f, 0.72f};
    const Vec3 closingColor{0.95f, 0.74f, 0.25f};
    const Vec3 cursorColor{1.0f, 0.22f, 0.22f};

    for (std::size_t index = 1; index < draft.points.size(); ++index) {
        addLine(vertices, draft.points[index - 1], draft.points[index], lineColor);
    }

    if (draft.points.size() >= 3) {
        addLine(vertices, draft.points.back(), draft.points.front(), closingColor);
    }

    for (const Vec3& point : draft.points) {
        appendPointMarker(vertices, point, lineColor);
    }

    appendPointMarker(vertices, draft.cursor, cursorColor);
    if (!draft.points.empty()) {
        addLine(vertices, draft.points.back(), draft.cursor, cursorColor);
    }
}

}  // namespace

std::vector<LineVertex> GuiScenarioRenderMapper::buildLineVertices(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft) const
{
    std::vector<LineVertex> vertices;
    vertices.reserve(scenario.planes.size() * 12 + draft.points.size() * 8 + 32);

    for (const GuiPlane& plane : scenario.planes) {
        appendPlaneLines(vertices, plane, plane.id == selection.selectedPlaneId);
    }

    appendDraftLines(vertices, draft);
    AxisGizmo::appendLines(vertices);
    PreparationSidePanel::appendLines(vertices, draft, selection.selectedPlaneId);
    return vertices;
}

}  // namespace gui
