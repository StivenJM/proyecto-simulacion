#include "PlanePropertiesPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <cstring>
#include <imgui.h>
#include <cstddef>
#include <string>

namespace gui {
namespace {

void separatorText(const char* label)
{
#if IMGUI_VERSION_NUM >= 18900
    ImGui::SeparatorText(label);
#else
    ImGui::TextUnformatted(label);
    ImGui::Separator();
#endif
}

}  // namespace

void PlanePropertiesPanel::render(PreparationScreen& screen)
{
    separatorText("Properties");

    const GuiPlane* plane = screen.selectedPlane();
    if (plane == nullptr) {
        ImGui::TextWrapped("Select a plane, source, or receiver in the scenario hierarchy to edit its properties.");
        return;
    }

    ImGui::Text("Plane #%d", plane->id);

    char nameBuffer[128]{};
    std::strncpy(nameBuffer, plane->name.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        screen.updateSelectedPlaneName(nameBuffer);
    }

    separatorText("Acoustic properties");
    ImGui::Text("Absorption: %.2f (%.0f%%)", plane->absorption, plane->absorption * 100.0f);
    ImGui::TextDisabled("0 = reflective, 1 = fully absorptive");

    float absorption = plane->absorption;
    if (ImGui::SliderFloat("Absorption coefficient", &absorption, 0.0f, 1.0f, "%.2f")) {
        screen.updateSelectedPlaneAbsorption(absorption);
    }
    ImGui::TextDisabled("This value is sent to the core simulation for this plane.");

    bool visible = plane->visible;
    if (ImGui::Checkbox("Visible", &visible)) {
        screen.updateSelectedPlaneVisibility(visible);
    }

    float color[3] = {plane->color.x, plane->color.y, plane->color.z};
    if (ImGui::ColorEdit3("Color", color)) {
        screen.updateSelectedPlaneColor({color[0], color[1], color[2]});
    }

    float normal[3] = {plane->normal.x, plane->normal.y, plane->normal.z};
    if (ImGui::InputFloat3("Normal", normal)) {
        screen.updateSelectedPlaneNormal({normal[0], normal[1], normal[2]});
    }
    ImGui::TextDisabled("Used by solid cube visualization to hide the face pointing toward the camera.");

    ImGui::Text("Area: %.2f m2", plane->area);

    if (ImGui::CollapsingHeader("Points")) {
        if (plane->outlinePoints.size() < 3) {
            ImGui::TextDisabled("This plane has no editable outline points.");
        } else {
            for (std::size_t index = 0; index < plane->outlinePoints.size(); ++index) {
                const Vec3& point = plane->outlinePoints[index];
                float values[3] = {point.x, point.y, point.z};
                const std::string label = "Point " + std::to_string(index + 1);
                if (ImGui::InputFloat3(label.c_str(), values)) {
                    screen.updateSelectedPlanePoint(index, {values[0], values[1], values[2]});
                }
            }
        }
    }

    ImGui::Text("Points: %zu", plane->outlinePoints.size());

    if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Triangles: %zu", plane->triangles.size());
        ImGui::TextDisabled("Simulated triangle fan; ready to be replaced by Core triangle output.");

        if (ImGui::BeginTable("TriangleTable", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Area");
            ImGui::TableSetupColumn("Centroid");
            ImGui::TableHeadersRow();

            for (const GuiTriangle& triangle : plane->triangles) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                const bool selected = triangle.id == screen.selectedTriangleId();
                const std::string label = std::to_string(triangle.id);
                if (ImGui::Selectable(label.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    screen.selectTriangle(triangle.id);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.2f", triangle.area);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("(%.2f, %.2f, %.2f)", triangle.centroid.x, triangle.centroid.y, triangle.centroid.z);
            }

            ImGui::EndTable();
        }

        if (const GuiTriangle* triangle = screen.selectedTriangle()) {
            separatorText("Selected triangle");
            ImGui::Text("ID: %d", triangle->id);
            ImGui::Text("Area: %.3f m2", triangle->area);
            ImGui::Text("Centroid: %.2f, %.2f, %.2f", triangle->centroid.x, triangle->centroid.y, triangle->centroid.z);
            ImGui::Text("Distance to plane center: %.2f m", triangle->distanceToPlaneCenter);
            for (std::size_t index = 0; index < triangle->vertices.size(); ++index) {
                const Vec3& vertex = triangle->vertices[index];
                ImGui::Text("V%zu: %.2f, %.2f, %.2f", index + 1, vertex.x, vertex.y, vertex.z);
            }
        }
    }
}

}  // namespace gui
