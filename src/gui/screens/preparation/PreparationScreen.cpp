#include "PreparationScreen.h"

#include "components/PlaneEditorControls.h"

#include <iostream>
#include <vector>

namespace gui {
namespace {

void move(Vec3& point, Vec3 delta)
{
    point.x += delta.x;
    point.y += delta.y;
    point.z += delta.z;
}

std::vector<Vec3> rectanglePointsForPlane(const GuiPlane& plane)
{
    if (!plane.outlinePoints.empty()) {
        return plane.outlinePoints;
    }

    const float halfWidth = plane.width / 2.0f;
    const float halfHeight = plane.height / 2.0f;

    if (plane.orientation == PlaneOrientation::Horizontal) {
        return {
            {plane.center.x - halfWidth, plane.center.y, plane.center.z - halfHeight},
            {plane.center.x + halfWidth, plane.center.y, plane.center.z - halfHeight},
            {plane.center.x + halfWidth, plane.center.y, plane.center.z + halfHeight},
            {plane.center.x - halfWidth, plane.center.y, plane.center.z + halfHeight},
        };
    }

    if (plane.orientation == PlaneOrientation::VerticalX) {
        return {
            {plane.center.x, plane.center.y - halfHeight, plane.center.z - halfWidth},
            {plane.center.x, plane.center.y - halfHeight, plane.center.z + halfWidth},
            {plane.center.x, plane.center.y + halfHeight, plane.center.z + halfWidth},
            {plane.center.x, plane.center.y + halfHeight, plane.center.z - halfWidth},
        };
    }

    return {
        {plane.center.x - halfWidth, plane.center.y - halfHeight, plane.center.z},
        {plane.center.x + halfWidth, plane.center.y - halfHeight, plane.center.z},
        {plane.center.x + halfWidth, plane.center.y + halfHeight, plane.center.z},
        {plane.center.x - halfWidth, plane.center.y + halfHeight, plane.center.z},
    };
}

const GuiPlane* findSelectedPlane(const GuiScenario& scenario, int planeId)
{
    for (const GuiPlane& plane : scenario.planes) {
        if (plane.id == planeId) {
            return &plane;
        }
    }

    return nullptr;
}

}  // namespace

PreparationScreen::PreparationScreen(GuiScenario& scenario, GuiSelection& selection, IPlaneService& planeService)
    : scenario_(scenario), selection_(selection), planeService_(planeService)
{
}

bool PreparationScreen::handleInput(const InputState& input, float deltaTime)
{
    bool titleChanged = false;

    if (PlaneEditorControls::newDraftPressed(input)) {
        draft_ = {};
        draft_.active = true;
        if (const GuiPlane* selectedPlane = findSelectedPlane(scenario_, selection_.selectedPlaneId)) {
            draft_.cursor = selectedPlane->center;
        }
        titleChanged = true;
        std::cout << "Started a new point-based plane draft. Move cursor with I/K/J/L/U/O, M adds a point, F finalizes.\n";
    }

    if (PlaneEditorControls::editSelectedPressed(input)) {
        if (const GuiPlane* selectedPlane = findSelectedPlane(scenario_, selection_.selectedPlaneId)) {
            draft_ = {};
            draft_.active = true;
            draft_.editingPlaneId = selectedPlane->id;
            draft_.points = rectanglePointsForPlane(*selectedPlane);
            draft_.cursor = draft_.points.empty() ? selectedPlane->center : draft_.points.back();
            titleChanged = true;
            std::cout << "Editing plane " << selectedPlane->id << " as a point draft.\n";
        } else {
            std::cout << "Select a plane before editing.\n";
        }
    }

    if (PlaneEditorControls::cancelDraftPressed(input) && draft_.active) {
        draft_ = {};
        titleChanged = true;
        std::cout << "Cancelled plane draft.\n";
    }

    const Vec3 delta = PlaneEditorControls::movementDelta(input, deltaTime);
    if (draft_.active && (delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)) {
        move(draft_.cursor, delta);
    }

    if (PlaneEditorControls::addDraftPointPressed(input) && draft_.active) {
        draft_.points.push_back(draft_.cursor);
        titleChanged = true;
        std::cout << "Added draft point " << draft_.points.size() << ". A valid plane needs at least 3 points.\n";
    }

    if (PlaneEditorControls::finalizeDraftPressed(input) && draft_.active) {
        if (!draft_.canFinalize()) {
            std::cout << "Plane draft needs at least 3 points before finalizing.\n";
        } else if (draft_.isEditingExistingPlane()) {
            if (planeService_.updatePlanePoints(scenario_, selection_, draft_.editingPlaneId, draft_.points)) {
                std::cout << "Updated plane " << selection_.selectedPlaneId << " from point draft.\n";
                draft_ = {};
                titleChanged = true;
            }
        } else {
            planeService_.addPlaneFromPoints(scenario_, selection_, draft_.points);
            std::cout << "Created point-based plane " << selection_.selectedPlaneId << ".\n";
            draft_ = {};
            titleChanged = true;
        }
    }

    if (PlaneEditorControls::addPlanePressed(input)) {
        planeService_.addPlane(scenario_, selection_);
        titleChanged = true;
        std::cout << "Added plane " << selection_.selectedPlaneId << ".\n";
    }

    if (PlaneEditorControls::addRoomPressed(input)) {
        planeService_.addRoom(scenario_, selection_);
        titleChanged = true;
        std::cout << "Added room planes. Selected plane " << selection_.selectedPlaneId << ".\n";
    }

    if (PlaneEditorControls::selectNextPressed(input)) {
        planeService_.selectNext(scenario_, selection_);
        titleChanged = true;
        std::cout << "Selected plane " << selection_.selectedPlaneId << ".\n";
    }

    if (!draft_.active && (delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)) {
        planeService_.moveSelected(scenario_, selection_, delta);
    }

    return titleChanged;
}

int PreparationScreen::selectedPlaneId() const
{
    return selection_.selectedPlaneId;
}

const GuiPlaneDraft& PreparationScreen::draft() const
{
    return draft_;
}

}  // namespace gui
