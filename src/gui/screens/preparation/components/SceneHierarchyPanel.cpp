#include "SceneHierarchyPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

namespace gui {

void SceneHierarchyPanel::render(PreparationScreen& screen)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float panelWidth = 260.0f;
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize({panelWidth, viewport->WorkSize.y}, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 16.0f));
    ImGui::Begin("Scenario", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Scenario");
    ImGui::TextDisabled("Select an object to edit its properties.");
    ImGui::Separator();

    if (ImGui::TreeNodeEx("Room", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (const GuiPlane& plane : screen.scenario().planes) {
            const bool selected = screen.selectedPlaneId() == plane.id;
            const std::string label = (plane.name.empty() ? "Plane" : plane.name) + "  #" + std::to_string(plane.id);
            if (ImGui::Selectable(label.c_str(), selected)) {
                screen.selectPlane(plane.id);
            }
        }

        if (screen.scenario().planes.empty()) {
            ImGui::TextDisabled("No planes yet.");
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Sources", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (screen.scenario().sources.empty()) {
            ImGui::TextDisabled("No sources yet.");
        } else {
            for (const GuiSource& source : screen.scenario().sources) {
                ImGui::TextDisabled("Source #%d", source.id);
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Receivers", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (screen.scenario().receivers.empty()) {
            ImGui::TextDisabled("No receivers yet.");
        } else {
            for (const GuiReceiver& receiver : screen.scenario().receivers) {
                ImGui::TextDisabled("Receiver #%d", receiver.id);
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace gui
