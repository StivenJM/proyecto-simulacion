#include "GeneralPropertiesPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

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

void GeneralPropertiesPanel::render(PreparationScreen& screen)
{
    separatorText("General configuration");
    ImGui::TextWrapped("Global simulation values used when the simulation starts.");

    int rayCount = screen.rayCount();
    if (ImGui::DragInt("Number of rays", &rayCount, 1.0f, 1, 10000)) {
        screen.updateRayCount(rayCount);
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        screen.updateRayCount(rayCount);
    }
    ImGui::TextDisabled("This value is sent to the core simulation as the global ray count.");

    ImGui::Separator();
    float absorption = screen.globalAbsorption();
    ImGui::Text("Global absorption: %.2f (%.0f%%)", absorption, absorption * 100.0f);
    ImGui::TextDisabled("Applies one absorption coefficient to every plane. Individual plane edits can still override it afterward.");
    if (ImGui::SliderFloat("All planes absorption", &absorption, 0.0f, 1.0f, "%.2f")) {
        screen.updateAllPlaneAbsorption(absorption);
    }
}

}  // namespace gui
