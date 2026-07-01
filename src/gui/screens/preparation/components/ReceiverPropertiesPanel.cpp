#include "ReceiverPropertiesPanel.h"

#include "gui/screens/preparation/PreparationScreen.h"

#include <cstring>
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

void ReceiverPropertiesPanel::render(PreparationScreen& screen)
{
    separatorText("Properties");

    const GuiReceiver* receiver = screen.selectedReceiver();
    if (receiver == nullptr) {
        ImGui::TextWrapped("Select a receiver in the scenario hierarchy to edit its properties.");
        return;
    }

    ImGui::Text("Receiver #%d", receiver->id);

    char nameBuffer[128]{};
    std::strncpy(nameBuffer, receiver->name.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        screen.updateSelectedReceiverName(nameBuffer);
    }

    bool visible = receiver->visible;
    if (ImGui::Checkbox("Visible", &visible)) {
        screen.updateSelectedReceiverVisibility(visible);
    }

    float position[3] = {receiver->position.x, receiver->position.y, receiver->position.z};
    if (ImGui::InputFloat3("Position", position)) {
        screen.updateSelectedReceiverPosition({position[0], position[1], position[2]});
    }

    ImGui::TextColored({receiver->color.x, receiver->color.y, receiver->color.z, 1.0f}, "Receiver icosahedron marker");
}

}  // namespace gui
