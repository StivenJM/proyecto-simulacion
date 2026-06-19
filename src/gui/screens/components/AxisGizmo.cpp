#include "AxisGizmo.h"

namespace gui {
namespace {

void addLine(std::vector<LineVertex>& vertices, Vec3 from, Vec3 to, Vec3 color)
{
    vertices.push_back({from, color});
    vertices.push_back({to, color});
}

}  // namespace

void AxisGizmo::appendLines(std::vector<LineVertex>& vertices)
{
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.25f, 0.25f});
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.25f, 1.0f, 0.25f});
    addLine(vertices, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.25f, 0.45f, 1.0f});
}

}  // namespace gui
