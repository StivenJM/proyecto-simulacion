#include "PreparationSidePanel.h"

namespace gui {
namespace {

void addLine(std::vector<LineVertex>& vertices, Vec3 from, Vec3 to, Vec3 color)
{
    vertices.push_back({from, color});
    vertices.push_back({to, color});
}

void appendCheckMark(std::vector<LineVertex>& vertices, Vec3 origin, Vec3 color)
{
    addLine(vertices, {origin.x, origin.y, origin.z}, {origin.x + 0.08f, origin.y - 0.08f, origin.z}, color);
    addLine(vertices, {origin.x + 0.08f, origin.y - 0.08f, origin.z}, {origin.x + 0.24f, origin.y + 0.12f, origin.z}, color);
}

void appendDash(std::vector<LineVertex>& vertices, Vec3 origin, Vec3 color)
{
    addLine(vertices, {origin.x, origin.y, origin.z}, {origin.x + 0.24f, origin.y, origin.z}, color);
}

}  // namespace

void PreparationSidePanel::appendLines(std::vector<LineVertex>& vertices, const GuiPlaneDraft& draft, int selectedPlaneId)
{
    const Vec3 frameColor{0.20f, 0.45f, 0.72f};
    const Vec3 activeColor{0.25f, 0.95f, 0.72f};
    const Vec3 pendingColor{0.95f, 0.74f, 0.25f};
    const Vec3 disabledColor{0.30f, 0.34f, 0.40f};

    const float x = 4.5f;
    const float yTop = 2.35f;
    const float yBottom = -2.35f;
    const float width = 1.65f;
    const float z = -1.2f;

    addLine(vertices, {x, yTop, z}, {x + width, yTop, z}, frameColor);
    addLine(vertices, {x + width, yTop, z}, {x + width, yBottom, z}, frameColor);
    addLine(vertices, {x + width, yBottom, z}, {x, yBottom, z}, frameColor);
    addLine(vertices, {x, yBottom, z}, {x, yTop, z}, frameColor);

    addLine(vertices, {x + 0.18f, yTop - 0.45f, z}, {x + width - 0.18f, yTop - 0.45f, z}, frameColor);

    const Vec3 draftColor = draft.active ? activeColor : disabledColor;
    const Vec3 validColor = draft.canFinalize() ? activeColor : pendingColor;
    appendCheckMark(vertices, {x + 0.22f, yTop - 0.82f, z}, draftColor);
    appendDash(vertices, {x + 0.22f, yTop - 1.18f, z}, validColor);
    appendDash(vertices, {x + 0.22f, yTop - 1.54f, z}, selectedPlaneId != 0 ? activeColor : disabledColor);

    const float pointSpacing = 0.16f;
    for (std::size_t index = 0; index < draft.points.size() && index < 12; ++index) {
        const float px = x + 0.28f + static_cast<float>(index % 4) * pointSpacing;
        const float py = yBottom + 0.42f + static_cast<float>(index / 4) * pointSpacing;
        addLine(vertices, {px - 0.035f, py, z}, {px + 0.035f, py, z}, pendingColor);
        addLine(vertices, {px, py - 0.035f, z}, {px, py + 0.035f, z}, pendingColor);
    }
}

}  // namespace gui
