#include "PlaneCreationPanel.h"

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

void PlaneCreationPanel::render(PreparationScreen& screen)
{
    separatorText("Plane Draft");

    const GuiPlaneDraft draft = screen.draft();
    ImGui::Text("Draft: ");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", draft.active ? (draft.isEditingExistingPlane() ? "Editing selected plane" : "New plane") : "Not started");
    ImGui::Text("Points: ");
    ImGui::SameLine();
    ImGui::TextDisabled("%zu / 3 minimum", screen.draftPointCount());

    if (ImGui::Button("Start Plane Draft", ImVec2(-1.0f, 0.0f))) {
        screen.startDraft();
    }

    const bool canEditSelected = screen.selectedPlaneId() != 0;
    if (!canEditSelected) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Edit Selected Plane", ImVec2(-1.0f, 0.0f))) {
        screen.editSelectedPlane();
    }
    if (!canEditSelected) {
        ImGui::EndDisabled();
    }

    Vec3 cursor = screen.draftCursor();
    float values[3] = {cursor.x, cursor.y, cursor.z};
    const bool hasActiveDraftForCursor = screen.hasActiveDraft();
    if (!hasActiveDraftForCursor) {
        ImGui::BeginDisabled();
    }
    if (ImGui::InputFloat3("Current Point", values)) {
        screen.setDraftCursor({values[0], values[1], values[2]});
    }
    if (ImGui::Button("Add Current Point", ImVec2(-1.0f, 0.0f))) {
        screen.addDraftPoint();
    }
    if (!hasActiveDraftForCursor) {
        ImGui::EndDisabled();
    }

    const bool canFinalizeDraft = screen.canFinalizeDraft();
    const bool editingExistingPlane = screen.draft().isEditingExistingPlane();
    if (!canFinalizeDraft) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(editingExistingPlane ? "Save Plane" : "Finalize Draft", ImVec2(-1.0f, 0.0f))) {
        screen.finalizeDraft();
    }
    if (!canFinalizeDraft) {
        ImGui::EndDisabled();
    }

    const bool hasActiveDraftForCancel = screen.hasActiveDraft();
    if (!hasActiveDraftForCancel) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Cancel Draft", ImVec2(-1.0f, 0.0f))) {
        screen.cancelDraft();
    }
    if (!hasActiveDraftForCancel) {
        ImGui::EndDisabled();
    }
}

}  // namespace gui
