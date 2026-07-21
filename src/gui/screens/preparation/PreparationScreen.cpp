#include "PreparationScreen.h"

#include "components/PlaneEditorControls.h"

#include <algorithm>
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

const GuiSource* findSelectedSource(const GuiScenario& scenario, int sourceId)
{
    for (const GuiSource& source : scenario.sources) {
        if (source.id == sourceId) {
            return &source;
        }
    }

    return nullptr;
}

const GuiReceiver* findSelectedReceiver(const GuiScenario& scenario, int receiverId)
{
    for (const GuiReceiver& receiver : scenario.receivers) {
        if (receiver.id == receiverId) {
            return &receiver;
        }
    }

    return nullptr;
}

const GuiTriangle* findSelectedTriangle(const GuiPlane* plane, int triangleId)
{
    if (plane == nullptr || triangleId == 0) {
        return nullptr;
    }

    for (const GuiTriangle& triangle : plane->triangles) {
        if (triangle.id == triangleId) {
            return &triangle;
        }
    }

    return nullptr;
}

constexpr int kMaxMeshSubdivisions = 10;

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

}  // namespace

PreparationScreen::PreparationScreen(
    GuiScenario& scenario,
    GuiSelection& selection,
    IPlaneService& planeService,
    ISourceService& sourceService,
    IReceiverService& receiverService
)
    : scenario_(scenario), selection_(selection), planeService_(planeService), sourceService_(sourceService), receiverService_(receiverService)
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
        if (selection_.isPlaneSelected()) {
            planeService_.moveSelected(scenario_, selection_, delta);
        } else if (selection_.isSourceSelected()) {
            sourceService_.moveSelected(scenario_, selection_, delta);
        } else if (selection_.isReceiverSelected()) {
            receiverService_.moveSelected(scenario_, selection_, delta);
        }
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

bool PreparationScreen::isGeneralSelected() const
{
    return selection_.isGeneralSelected();
}

int PreparationScreen::selectedPlaneId() const
{
    return selection_.selectedPlaneId();
}

int PreparationScreen::selectedSourceId() const
{
    return selection_.selectedSourceId();
}

int PreparationScreen::selectedReceiverId() const
{
    return selection_.selectedReceiverId();
}

int PreparationScreen::selectedTriangleId() const
{
    return selection_.selectedTriangleId();
}

void PreparationScreen::selectGeneral()
{
    selection_.selectGeneral();
}

void PreparationScreen::selectPlane(int planeId)
{
    selection_.selectPlane(planeId);
}

void PreparationScreen::selectSource(int sourceId)
{
    selection_.selectSource(sourceId);
}

void PreparationScreen::selectReceiver(int receiverId)
{
    selection_.selectReceiver(receiverId);
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

const GuiSource* PreparationScreen::selectedSource() const
{
    return findSelectedSource(scenario_, selection_.selectedSourceId());
}

const GuiReceiver* PreparationScreen::selectedReceiver() const
{
    return findSelectedReceiver(scenario_, selection_.selectedReceiverId());
}

const GuiTriangle* PreparationScreen::selectedTriangle() const
{
    return findSelectedTriangle(selectedPlane(), selection_.selectedTriangleId());
}

void PreparationScreen::addDefaultPlane()
{
    planeService_.addPlane(scenario_, selection_);
}

void PreparationScreen::addDefaultSource()
{
    sourceService_.addSource(scenario_, selection_);
}

void PreparationScreen::addDefaultReceiver()
{
    receiverService_.addReceiver(scenario_, selection_);
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

void PreparationScreen::updateAllPlaneAbsorption(float absorption)
{
    planeService_.updateAllPlaneAbsorption(scenario_, absorption);
}

bool PreparationScreen::updateSelectedPlaneVisibility(bool visible)
{
    return planeService_.updatePlaneVisibility(scenario_, selectedPlaneId(), visible);
}

bool PreparationScreen::updateSelectedPlaneColor(Vec3 color)
{
    return planeService_.updatePlaneColor(scenario_, selectedPlaneId(), color);
}

bool PreparationScreen::updateSelectedPlaneNormal(Vec3 normal)
{
    return planeService_.updatePlaneNormal(scenario_, selectedPlaneId(), normal);
}

void PreparationScreen::selectTriangle(int triangleId)
{
    selection_.selectTriangle(triangleId);
}

bool PreparationScreen::updateSelectedSourcePosition(Vec3 position)
{
    return sourceService_.updateSourcePosition(scenario_, selectedSourceId(), position);
}

bool PreparationScreen::updateSelectedSourceName(const std::string& name)
{
    return sourceService_.updateSourceName(scenario_, selectedSourceId(), name);
}

bool PreparationScreen::updateSelectedSourceEnergy(float energy)
{
    return sourceService_.updateSourceEnergy(scenario_, selectedSourceId(), energy);
}

bool PreparationScreen::updateSelectedSourceVisibility(bool visible)
{
    return sourceService_.updateSourceVisibility(scenario_, selectedSourceId(), visible);
}

bool PreparationScreen::updateSelectedReceiverPosition(Vec3 position)
{
    return receiverService_.updateReceiverPosition(scenario_, selectedReceiverId(), position);
}

bool PreparationScreen::updateSelectedReceiverName(const std::string& name)
{
    return receiverService_.updateReceiverName(scenario_, selectedReceiverId(), name);
}

bool PreparationScreen::updateSelectedReceiverVisibility(bool visible)
{
    return receiverService_.updateReceiverVisibility(scenario_, selectedReceiverId(), visible);
}

int PreparationScreen::rayCount() const
{
    return scenario_.simulationConfig.rayCount;
}

void PreparationScreen::updateRayCount(int rayCount)
{
    scenario_.simulationConfig.rayCount = rayCount < 1 ? 1 : rayCount;
}

int PreparationScreen::meshSubdivisions() const
{
    return scenario_.simulationConfig.meshSubdivisions;
}

void PreparationScreen::updateMeshSubdivisions(int meshSubdivisions)
{
    scenario_.simulationConfig.meshSubdivisions = std::clamp(meshSubdivisions, 1, kMaxMeshSubdivisions);
}

float PreparationScreen::globalAbsorption() const
{
    return scenario_.simulationConfig.globalAbsorption;
}

float PreparationScreen::diffusionCoefficient() const
{
    return scenario_.simulationConfig.diffusionCoefficient;
}

void PreparationScreen::updateDiffusionCoefficient(float diffusionCoefficient)
{
    scenario_.simulationConfig.diffusionCoefficient = clamp01(diffusionCoefficient);
}

bool PreparationScreen::solidSceneFill() const
{
    return scenario_.simulationConfig.solidSceneFill;
}

void PreparationScreen::updateSolidSceneFill(bool solidSceneFill)
{
    scenario_.simulationConfig.solidSceneFill = solidSceneFill;
}

const GuiPlaneDraft& PreparationScreen::draft() const
{
    return draft_;
}

}  // namespace gui
