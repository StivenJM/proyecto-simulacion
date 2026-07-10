#include "SceneHierarchyPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

#include <cstdio>
#include <string>

namespace gui {
namespace {

struct SectionHeaderState {
    bool open = false;
    bool addClicked = false;
};

SectionHeaderState sectionHeader(const char* label, const char* tooltip)
{
    ImGui::PushID(label);
    const bool open = ImGui::TreeNodeEx("##section", ImGuiTreeNodeFlags_DefaultOpen, "%s", label);

    const float buttonSize = ImGui::GetFrameHeight();
    const float buttonX = ImGui::GetWindowContentRegionMax().x - buttonSize;
    ImGui::SameLine(buttonX);
    const bool addClicked = ImGui::Button("+", ImVec2(buttonSize, 0.0f));
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    ImGui::PopID();

    return {open, addClicked};
}

}  // namespace

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

    if (ImGui::Selectable("General", screen.isGeneralSelected())) {
        screen.selectGeneral();
    }

    const SectionHeaderState planesHeader = sectionHeader("Planes", "Add plane");
    if (planesHeader.addClicked) {
        screen.addDefaultPlane();
    }
    if (planesHeader.open) {
        for (const GuiPlane& plane : screen.scenario().planes) {
            const bool selected = screen.selectedPlaneId() == plane.id;
            char absorptionLabel[32]{};
            std::snprintf(absorptionLabel, sizeof(absorptionLabel), "  a=%.2f", plane.absorption);
            const std::string label = (plane.name.empty() ? "Plane" : plane.name) + "  #" + std::to_string(plane.id) + absorptionLabel;
            if (ImGui::Selectable(label.c_str(), selected)) {
                screen.selectPlane(plane.id);
            }
        }

        if (screen.scenario().planes.empty()) {
            ImGui::TextDisabled("No planes yet.");
        }
        ImGui::TreePop();
    }

    const SectionHeaderState sourcesHeader = sectionHeader("Sources", "Add source");
    if (sourcesHeader.addClicked) {
        screen.addDefaultSource();
    }
    if (sourcesHeader.open) {
        if (screen.scenario().sources.empty()) {
            ImGui::TextDisabled("No sources yet.");
        } else {
            for (const GuiSource& source : screen.scenario().sources) {
                const bool selected = screen.selectedSourceId() == source.id;
                const std::string label = (source.name.empty() ? "Source" : source.name) + "  #" + std::to_string(source.id);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    screen.selectSource(source.id);
                }
            }
        }
        ImGui::TreePop();
    }

    const SectionHeaderState receiversHeader = sectionHeader("Receivers", "Add receiver");
    if (receiversHeader.addClicked) {
        screen.addDefaultReceiver();
    }
    if (receiversHeader.open) {
        if (screen.scenario().receivers.empty()) {
            ImGui::TextDisabled("No receivers yet.");
        } else {
            for (const GuiReceiver& receiver : screen.scenario().receivers) {
                const bool selected = screen.selectedReceiverId() == receiver.id;
                const std::string label = (receiver.name.empty() ? "Receiver" : receiver.name) + "  #" + std::to_string(receiver.id);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    screen.selectReceiver(receiver.id);
                }
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace gui
