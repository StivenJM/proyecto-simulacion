#include "PlaneEditorPanel.h"

#include "GeneralPropertiesPanel.h"
#include "PlaneCreationPanel.h"
#include "PlanePropertiesPanel.h"
#include "ReceiverPropertiesPanel.h"
#include "SourcePropertiesPanel.h"
#include "gui/screens/preparation/PreparationScreen.h"

#include <imgui.h>

namespace gui {
void PlaneEditorPanel::render(PreparationScreen& screen)
{
    PlaneCreationPanel creationPanel;
    GeneralPropertiesPanel generalPropertiesPanel;
    PlanePropertiesPanel propertiesPanel;
    ReceiverPropertiesPanel receiverPropertiesPanel;
    SourcePropertiesPanel sourcePropertiesPanel;

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
    ImGui::TextDisabled("Selected scenario object details.");

    if (screen.isGeneralSelected()) {
        generalPropertiesPanel.render(screen);
    } else if (screen.selectedSource() != nullptr) {
        sourcePropertiesPanel.render(screen);
    } else if (screen.selectedReceiver() != nullptr) {
        receiverPropertiesPanel.render(screen);
    } else {
        propertiesPanel.render(screen);

        if (screen.selectedPlane() != nullptr || screen.hasActiveDraft()) {
            ImGui::Spacing();
            creationPanel.render(screen);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

}  // namespace gui
