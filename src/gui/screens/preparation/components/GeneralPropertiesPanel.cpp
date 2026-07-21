#include "GeneralPropertiesPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

#include <algorithm>

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
    constexpr int kMaxMeshSubdivisions = 10;

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

    int meshSubdivisions = screen.meshSubdivisions();
    if (ImGui::DragInt("Mesh subdivisions", &meshSubdivisions, 1.0f, 1, kMaxMeshSubdivisions)) {
        screen.updateMeshSubdivisions(std::clamp(meshSubdivisions, 1, kMaxMeshSubdivisions));
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        screen.updateMeshSubdivisions(std::clamp(meshSubdivisions, 1, kMaxMeshSubdivisions));
    }
    ImGui::TextDisabled("Core simulation builds an N x N mesh for four-point planes. Maximum: 10.");

    ImGui::Separator();
    bool solidSceneFill = screen.solidSceneFill();
    if (ImGui::Checkbox("Solid cube visualization", &solidSceneFill)) {
        screen.updateSolidSceneFill(solidSceneFill);
    }
    ImGui::TextDisabled("Switches plane fills between the default translucent view and a solid cube view.");

    ImGui::Separator();
    float absorption = screen.globalAbsorption();
    ImGui::Text("Global absorption: %.2f (%.0f%%)", absorption, absorption * 100.0f);
    ImGui::TextDisabled("Applies one absorption coefficient to every plane. Individual plane edits can still override it afterward.");
    if (ImGui::SliderFloat("All planes absorption", &absorption, 0.0f, 1.0f, "%.2f")) {
        screen.updateAllPlaneAbsorption(absorption);
    }

    float diffusion = screen.diffusionCoefficient();
    ImGui::Text("Diffusion coefficient: %.2f (%.0f%%)", diffusion, diffusion * 100.0f);
    ImGui::TextDisabled("Controls how much post-absorption energy is sent to diffuse propagation.");
    if (ImGui::SliderFloat("Diffusion coefficient", &diffusion, 0.0f, 1.0f, "%.2f")) {
        screen.updateDiffusionCoefficient(diffusion);
    }
}

}  // namespace gui
