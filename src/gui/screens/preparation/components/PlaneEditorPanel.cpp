#include "PlaneEditorPanel.h"

#include "PlaneCreationPanel.h"
#include "PlanePropertiesPanel.h"
#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

namespace gui {
void PlaneEditorPanel::render(PreparationScreen& screen)
{
    PlaneCreationPanel creationPanel;
    PlanePropertiesPanel propertiesPanel;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float panelWidth = 360.0f;
    ImGui::SetNextWindowPos({viewport->WorkPos.x + viewport->WorkSize.x - panelWidth, viewport->WorkPos.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({panelWidth, viewport->WorkSize.y}, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 18.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 12.0f));
    ImGui::Begin("Properties", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Object Properties");
    ImGui::TextDisabled("Scenario object details and creation tools.");
    propertiesPanel.render(screen);
    ImGui::Spacing();
    creationPanel.render(screen);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

}  // namespace gui
