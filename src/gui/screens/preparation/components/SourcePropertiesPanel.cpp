#include "SourcePropertiesPanel.h"

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

void SourcePropertiesPanel::render(PreparationScreen& screen)
{
    separatorText("Properties");

    const GuiSource* source = screen.selectedSource();
    if (source == nullptr) {
        ImGui::TextWrapped("Select a source in the scenario hierarchy to edit its properties.");
        return;
    }

    ImGui::Text("Source #%d", source->id);

    char nameBuffer[128]{};
    std::strncpy(nameBuffer, source->name.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        screen.updateSelectedSourceName(nameBuffer);
    }

    bool visible = source->visible;
    if (ImGui::Checkbox("Visible", &visible)) {
        screen.updateSelectedSourceVisibility(visible);
    }

    float energy = source->energy;
    if (ImGui::DragFloat("Energy", &energy, 0.1f, 0.0f, 1000.0f, "%.2f")) {
        screen.updateSelectedSourceEnergy(energy);
    }

    float position[3] = {source->position.x, source->position.y, source->position.z};
    if (ImGui::InputFloat3("Position", position)) {
        screen.updateSelectedSourcePosition({position[0], position[1], position[2]});
    }

    ImGui::TextColored({source->color.x, source->color.y, source->color.z, 1.0f}, "Source icosahedron marker");
}

}  // namespace gui
