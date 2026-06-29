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
    ImGui::TextDisabled("Simulation result impact will use this value once the solver is connected.");

    bool visible = plane->visible;
    if (ImGui::Checkbox("Visible", &visible)) {
        screen.updateSelectedPlaneVisibility(visible);
    }

    float color[3] = {plane->color.x, plane->color.y, plane->color.z};
    if (ImGui::ColorEdit3("Color", color)) {
        screen.updateSelectedPlaneColor({color[0], color[1], color[2]});
    }

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
    ImGui::TextDisabled("Triangles: pending mesh generation");
}

}  // namespace gui
