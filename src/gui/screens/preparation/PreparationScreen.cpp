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
        startDraft();
        titleChanged = true;
    }

    if (PlaneEditorControls::editSelectedPressed(input)) {
        titleChanged = editSelectedPlane();
    }

    if (PlaneEditorControls::cancelDraftPressed(input) && draft_.active) {
        cancelDraft();
        titleChanged = true;
    }

    const Vec3 delta = PlaneEditorControls::movementDelta(input, deltaTime);
    if (draft_.active && (delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)) {
        move(draft_.cursor, delta);
    }

    if (PlaneEditorControls::addDraftPointPressed(input) && draft_.active) {
        addDraftPoint();
        titleChanged = true;
    }

    if (PlaneEditorControls::finalizeDraftPressed(input) && draft_.active) {
        titleChanged = finalizeDraft();
    }

    if (PlaneEditorControls::addPlanePressed(input)) {
        addDefaultPlane();
        titleChanged = true;
        std::cout << "Added plane " << selection_.selectedPlaneId() << ".\n";
    }

    if (PlaneEditorControls::addRoomPressed(input)) {
        planeService_.addRoom(scenario_, selection_);
        titleChanged = true;
        std::cout << "Added room planes. Selected plane " << selection_.selectedPlaneId() << ".\n";
    }

    if (PlaneEditorControls::selectNextPressed(input)) {
        planeService_.selectNext(scenario_, selection_);
        titleChanged = true;
        std::cout << "Selected plane " << selection_.selectedPlaneId() << ".\n";
    }

    if (!draft_.active && (delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)) {
        planeService_.moveSelected(scenario_, selection_, delta);
    }

    return titleChanged;
}

void PreparationScreen::startDraft()
{
    draft_ = {};
    draft_.active = true;
    if (const GuiPlane* selectedPlane = findSelectedPlane(scenario_, selection_.selectedPlaneId())) {
        draft_.cursor = selectedPlane->center;
    }
    std::cout << "Started a new point-based plane draft. Move cursor with I/K/J/L/U/O, M adds a point, F finalizes.\n";
}

void PreparationScreen::addDraftPoint()
{
    if (!draft_.active) {
        return;
    }

    draft_.points.push_back(draft_.cursor);
    std::cout << "Added draft point " << draft_.points.size() << ". A valid plane needs at least 3 points.\n";
}

bool PreparationScreen::finalizeDraft()
{
    if (!draft_.active) {
        return false;
    }

    if (!draft_.canFinalize()) {
        std::cout << "Plane draft needs at least 3 points before finalizing.\n";
        return false;
    }

    if (draft_.isEditingExistingPlane()) {
        if (planeService_.updatePlanePoints(scenario_, selection_, draft_.editingPlaneId, draft_.points)) {
            std::cout << "Updated plane " << selection_.selectedPlaneId() << " from point draft.\n";
            draft_ = {};
            return true;
        }

        return false;
    }

    planeService_.addPlaneFromPoints(scenario_, selection_, draft_.points);
    std::cout << "Created point-based plane " << selection_.selectedPlaneId() << ".\n";
    draft_ = {};
    return true;
}

bool PreparationScreen::editSelectedPlane()
{
    if (const GuiPlane* selectedPlane = findSelectedPlane(scenario_, selection_.selectedPlaneId())) {
        draft_ = {};
        draft_.active = true;
        draft_.editingPlaneId = selectedPlane->id;
        draft_.points = rectanglePointsForPlane(*selectedPlane);
        draft_.cursor = draft_.points.empty() ? selectedPlane->center : draft_.points.back();
        std::cout << "Editing plane " << selectedPlane->id << " as a point draft.\n";
        return true;
    }

    std::cout << "Select a plane before editing.\n";
    return false;
}

void PreparationScreen::cancelDraft()
{
    if (!draft_.active) {
        return;
    }

    draft_ = {};
    std::cout << "Cancelled plane draft.\n";
}

void PreparationScreen::setDraftCursor(Vec3 cursor)
{
    draft_.cursor = cursor;
}

Vec3 PreparationScreen::draftCursor() const
{
    return draft_.cursor;
}

std::size_t PreparationScreen::draftPointCount() const
{
    return draft_.points.size();
}

bool PreparationScreen::canFinalizeDraft() const
{
    return draft_.canFinalize();
}

bool PreparationScreen::hasActiveDraft() const
{
    return draft_.active;
}

int PreparationScreen::selectedPlaneId() const
{
    return selection_.selectedPlaneId();
}

void PreparationScreen::selectPlane(int planeId)
{
    selection_.selectPlane(planeId);
}

GuiScenario& PreparationScreen::scenario()
{
    return scenario_;
}

const GuiScenario& PreparationScreen::scenario() const
{
    return scenario_;
}

GuiSelection& PreparationScreen::selection()
{
    return selection_;
}

const GuiPlane* PreparationScreen::selectedPlane() const
{
    return findSelectedPlane(scenario_, selection_.selectedPlaneId());
}

void PreparationScreen::addDefaultPlane()
{
    planeService_.addPlane(scenario_, selection_);
}

bool PreparationScreen::updateSelectedPlanePoint(std::size_t pointIndex, Vec3 point)
{
    return planeService_.updatePlanePoint(scenario_, selection_, selectedPlaneId(), pointIndex, point);
}

bool PreparationScreen::updateSelectedPlaneName(const std::string& name)
{
    return planeService_.updatePlaneName(scenario_, selectedPlaneId(), name);
}

bool PreparationScreen::updateSelectedPlaneAbsorption(float absorption)
{
    return planeService_.updatePlaneAbsorption(scenario_, selectedPlaneId(), absorption);
}

bool PreparationScreen::updateSelectedPlaneVisibility(bool visible)
{
    return planeService_.updatePlaneVisibility(scenario_, selectedPlaneId(), visible);
}

bool PreparationScreen::updateSelectedPlaneColor(Vec3 color)
{
    return planeService_.updatePlaneColor(scenario_, selectedPlaneId(), color);
}

const GuiPlaneDraft& PreparationScreen::draft() const
{
    return draft_;
}

}  // namespace gui
