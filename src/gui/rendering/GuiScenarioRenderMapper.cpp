#include "GuiScenarioRenderMapper.h"

#include "gui/screens/components/AxisGizmo.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unordered_map>

namespace gui {
namespace {

constexpr int NUM_COLORS = 4;

class color {
public:
    double R = 1.0;
    double G = 1.0;
    double B = 1.0;

    color operator*(double f)
    {
        color c;
        c.R = R * f;
        c.G = G * f;
        c.B = B * f;
        return c;
    }

    void operator=(double f)
    {
        R = G = B = f;
    }

    void getHeatMapColor(double value)
    {
        color arr_col[NUM_COLORS];

        arr_col[0].R = 0.5;
        arr_col[0].G = 0.5;
        arr_col[0].B = 1.0;

        arr_col[1].R = 0.5;
        arr_col[1].G = 1.0;
        arr_col[1].B = 0.5;

        arr_col[2].R = 1.0;
        arr_col[2].G = 1.0;
        arr_col[2].B = 0.5;

        arr_col[3].R = 1.0;
        arr_col[3].G = 0.5;
        arr_col[3].B = 0.5;

        int ind1;
        int ind2;
        double fracIntermedia = 0.0;

        if (value <= 0.0) {
            ind1 = ind2 = 0;
        } else if (value >= 1.0) {
            ind1 = ind2 = 3;
        } else {
            value = value * (NUM_COLORS - 1);
            ind1 = static_cast<int>(std::floor(value));
            ind2 = ind1 + 1;
            fracIntermedia = value - static_cast<double>(ind1);
        }

        R = (arr_col[ind2].R - arr_col[ind1].R) * fracIntermedia + arr_col[ind1].R;
        G = (arr_col[ind2].G - arr_col[ind1].G) * fracIntermedia + arr_col[ind1].G;
        B = (arr_col[ind2].B - arr_col[ind1].B) * fracIntermedia + arr_col[ind1].B;
    }
};

void addLine(std::vector<LineVertex>& vertices, Vec3 from, Vec3 to, Vec3 color)
{
    vertices.push_back({from, color});
    vertices.push_back({to, color});
}

void addTriangle(std::vector<ColoredVertex>& vertices, Vec3 a, Vec3 b, Vec3 c, ColorRgba color)
{
    vertices.push_back({a, color});
    vertices.push_back({b, color});
    vertices.push_back({c, color});
}

Vec3 darker(Vec3 color)
{
    constexpr float factor = 0.45f;
    return {color.x * factor, color.y * factor, color.z * factor};
}

Vec3 selectedMarkerColor(Vec3 color)
{
    constexpr float highlight = 0.22f;
    return {
        color.x + (1.0f - color.x) * highlight,
        color.y + (1.0f - color.y) * highlight,
        color.z + (1.0f - color.z) * highlight,
    };
}

float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

float mix(float from, float to, float amount)
{
    return from + (to - from) * amount;
}

Vec3 mix(Vec3 from, Vec3 to, float amount)
{
    return {
        mix(from.x, to.x, amount),
        mix(from.y, to.y, amount),
        mix(from.z, to.z, amount),
    };
}

ColorRgba absorptionMappedPlaneFillColor(const GuiPlane& plane)
{
    const float absorption = clamp01(plane.absorption);
    const Vec3 reflectiveHighlight = mix(plane.color, Vec3{0.72f, 0.88f, 1.0f}, 0.28f);
    const Vec3 absorptiveTint = mix(plane.color, Vec3{1.0f, 0.42f, 0.18f}, 0.45f);
    const Vec3 mappedColor = mix(reflectiveHighlight, absorptiveTint, absorption);
    return {mappedColor.x, mappedColor.y, mappedColor.z, 0.30f};
}

struct TriangleEnergyKey {
    int planeId{};
    int triangleId{};

    bool operator==(const TriangleEnergyKey& other) const
    {
        return planeId == other.planeId && triangleId == other.triangleId;
    }
};

struct TriangleEnergyKeyHash {
    std::size_t operator()(const TriangleEnergyKey& key) const
    {
        const std::size_t planeHash = std::hash<int>{}(key.planeId);
        const std::size_t triangleHash = std::hash<int>{}(key.triangleId);
        return planeHash ^ (triangleHash + 0x9e3779b9u + (planeHash << 6u) + (planeHash >> 2u));
    }
};

using TriangleEnergyLookup = std::unordered_map<TriangleEnergyKey, float, TriangleEnergyKeyHash>;

TriangleEnergyLookup buildTriangleEnergyLookup(const RenderSimulationOverlay& overlay)
{
    TriangleEnergyLookup lookup;
    if (!overlay.active || !overlay.showDiffuseEnergy) {
        return lookup;
    }

    lookup.reserve(overlay.triangleEnergy.size());
    for (const RenderTriangleEnergy& sample : overlay.triangleEnergy) {
        const TriangleEnergyKey key{sample.planeId, sample.triangleId};
        const float energy = clamp01(sample.energy);
        auto it = lookup.find(key);
        if (it == lookup.end()) {
            lookup.emplace(key, energy);
        } else {
            it->second = std::max(it->second, energy);
        }
    }

    return lookup;
}

float planeEnergy(const RenderSimulationOverlay& overlay, int planeId)
{
    if (!overlay.active || !overlay.showDiffuseEnergy) {
        return 0.0f;
    }

    for (const RenderPlaneEnergy& sample : overlay.planeEnergy) {
        if (sample.planeId == planeId) {
            return clamp01(sample.energy);
        }
    }

    return 0.0f;
}

float triangleEnergy(const TriangleEnergyLookup& lookup, int planeId, int triangleId, float fallback)
{
    auto it = lookup.find({planeId, triangleId});
    if (it != lookup.end()) {
        return it->second;
    }

    return fallback;
}

ColorRgba energyTintedFillColor(const GuiPlane& plane, float energy)
{
    (void)plane;
    color heatMapColor;
    const float mappedEnergy = clamp01(energy);
    heatMapColor.getHeatMapColor(mappedEnergy);
    const float alpha = mappedEnergy > 0.0f ? 0.74f : 0.34f;
    return {static_cast<float>(heatMapColor.R), static_cast<float>(heatMapColor.G), static_cast<float>(heatMapColor.B), alpha};
}

bool hasOverlayTriangleGeometry(const RenderSimulationOverlay& simulationOverlay, int planeId)
{
    if (!simulationOverlay.active || !simulationOverlay.showDiffuseEnergy) {
        return false;
    }

    return std::any_of(simulationOverlay.triangleEnergy.begin(), simulationOverlay.triangleEnergy.end(), [planeId](const RenderTriangleEnergy& sample) {
        return sample.planeId == planeId && sample.hasGeometry;
    });
}

void appendOverlayTriangleEnergy(std::vector<ColoredVertex>& vertices, const RenderSimulationOverlay& simulationOverlay, int planeId)
{
    for (const RenderTriangleEnergy& sample : simulationOverlay.triangleEnergy) {
        if (sample.planeId != planeId || !sample.hasGeometry) {
            continue;
        }

        addTriangle(vertices, sample.vertices[0], sample.vertices[1], sample.vertices[2], energyTintedFillColor({}, sample.energy));
    }
}

void appendPlaneLines(std::vector<LineVertex>& vertices, const GuiPlane& plane, bool selected)
{
    if (!plane.visible) {
        return;
    }

    const Vec3 color = selected ? Vec3{1.0f, 0.86f, 0.18f} : plane.color;

    if (plane.outlinePoints.size() >= 3) {
        for (std::size_t index = 0; index < plane.outlinePoints.size(); ++index) {
            const Vec3 from = plane.outlinePoints[index];
            const Vec3 to = plane.outlinePoints[(index + 1) % plane.outlinePoints.size()];
            addLine(vertices, from, to, color);
        }

        if (selected) {
            addLine(vertices, plane.center, plane.outlinePoints.front(), color);
        }

        return;
    }

    const float halfWidth = plane.width / 2.0f;
    const float halfHeight = plane.height / 2.0f;

    Vec3 a{};
    Vec3 b{};
    Vec3 c{};
    Vec3 d{};

    if (plane.orientation == PlaneOrientation::Horizontal) {
        a = {plane.center.x - halfWidth, plane.center.y, plane.center.z - halfHeight};
        b = {plane.center.x + halfWidth, plane.center.y, plane.center.z - halfHeight};
        c = {plane.center.x + halfWidth, plane.center.y, plane.center.z + halfHeight};
        d = {plane.center.x - halfWidth, plane.center.y, plane.center.z + halfHeight};
    } else if (plane.orientation == PlaneOrientation::VerticalX) {
        a = {plane.center.x, plane.center.y - halfHeight, plane.center.z - halfWidth};
        b = {plane.center.x, plane.center.y - halfHeight, plane.center.z + halfWidth};
        c = {plane.center.x, plane.center.y + halfHeight, plane.center.z + halfWidth};
        d = {plane.center.x, plane.center.y + halfHeight, plane.center.z - halfWidth};
    } else {
        a = {plane.center.x - halfWidth, plane.center.y - halfHeight, plane.center.z};
        b = {plane.center.x + halfWidth, plane.center.y - halfHeight, plane.center.z};
        c = {plane.center.x + halfWidth, plane.center.y + halfHeight, plane.center.z};
        d = {plane.center.x - halfWidth, plane.center.y + halfHeight, plane.center.z};
    }

    addLine(vertices, a, b, color);
    addLine(vertices, b, c, color);
    addLine(vertices, c, d, color);
    addLine(vertices, d, a, color);

    if (selected) {
        addLine(vertices, a, c, color);
        addLine(vertices, b, d, color);
    }
}

void appendPlaneFill(std::vector<ColoredVertex>& vertices, const GuiPlane& plane, const RenderSimulationOverlay& simulationOverlay, const TriangleEnergyLookup& triangleEnergyLookup)
{
    if (!plane.visible || plane.outlinePoints.size() < 3) {
        return;
    }

    const bool showDiffuseEnergy = simulationOverlay.active && simulationOverlay.showDiffuseEnergy;
    if (showDiffuseEnergy && hasOverlayTriangleGeometry(simulationOverlay, plane.id)) {
        appendOverlayTriangleEnergy(vertices, simulationOverlay, plane.id);
        return;
    }

    const float planeLevel = planeEnergy(simulationOverlay, plane.id);
    if (showDiffuseEnergy && !plane.triangles.empty()) {
        for (const GuiTriangle& triangle : plane.triangles) {
            if (!triangle.visible) {
                continue;
            }

            const float level = triangleEnergy(triangleEnergyLookup, plane.id, triangle.id, planeLevel);
            addTriangle(vertices, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], energyTintedFillColor(plane, level));
        }
        return;
    }

    const ColorRgba color = showDiffuseEnergy ? energyTintedFillColor(plane, planeLevel) : absorptionMappedPlaneFillColor(plane);
    const Vec3 origin = plane.outlinePoints.front();
    for (std::size_t index = 1; index + 1 < plane.outlinePoints.size(); ++index) {
        vertices.push_back({origin, color});
        vertices.push_back({plane.outlinePoints[index], color});
        vertices.push_back({plane.outlinePoints[index + 1], color});
    }
}

void appendPointMarker(std::vector<LineVertex>& vertices, Vec3 point, Vec3 color)
{
    constexpr float size = 0.06f;
    addLine(vertices, {point.x - size, point.y, point.z}, {point.x + size, point.y, point.z}, color);
    addLine(vertices, {point.x, point.y - size, point.z}, {point.x, point.y + size, point.z}, color);
    addLine(vertices, {point.x, point.y, point.z - size}, {point.x, point.y, point.z + size}, color);
}

void appendTriangleInspection(std::vector<ColoredVertex>& fillVertices, std::vector<LineVertex>& lineVertices, const GuiPlane& plane, int selectedTriangleId)
{
    if (!plane.visible) {
        return;
    }

    const Vec3 edgeColor{1.0f, 0.86f, 0.18f};
    const Vec3 centroidColor{0.22f, 1.0f, 0.74f};
    const Vec3 selectedEdgeColor{1.0f, 0.34f, 0.18f};
    const ColorRgba selectedFillColor{1.0f, 0.34f, 0.18f, 0.36f};

    for (const GuiTriangle& triangle : plane.triangles) {
        if (!triangle.visible) {
            continue;
        }

        const bool selected = triangle.id == selectedTriangleId;
        const Vec3 color = selected ? selectedEdgeColor : edgeColor;
        addLine(lineVertices, triangle.vertices[0], triangle.vertices[1], color);
        addLine(lineVertices, triangle.vertices[1], triangle.vertices[2], color);
        addLine(lineVertices, triangle.vertices[2], triangle.vertices[0], color);
        appendPointMarker(lineVertices, triangle.centroid, selected ? selectedEdgeColor : centroidColor);

        if (selected) {
            addTriangle(fillVertices, triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], selectedFillColor);
        }
    }
}

std::array<Vec3, 12> buildIcosahedronPoints(Vec3 center, float radius)
{
    constexpr float ringZ = 0.44721359550f;
    constexpr float ringRadius = 0.89442719100f;

    const std::array<Vec3, 12> unitPoints{
        Vec3{0.0f, 0.0f, 1.0f},
        Vec3{0.0f, ringRadius, ringZ},
        Vec3{0.85065080835f, 0.27639320225f, ringZ},
        Vec3{0.52573111212f, -0.72360679775f, ringZ},
        Vec3{-0.52573111212f, -0.72360679775f, ringZ},
        Vec3{-0.85065080835f, 0.27639320225f, ringZ},
        Vec3{0.52573111212f, 0.72360679775f, -ringZ},
        Vec3{0.85065080835f, -0.27639320225f, -ringZ},
        Vec3{0.0f, -ringRadius, -ringZ},
        Vec3{-0.85065080835f, -0.27639320225f, -ringZ},
        Vec3{-0.52573111212f, 0.72360679775f, -ringZ},
        Vec3{0.0f, 0.0f, -1.0f},
    };

    std::array<Vec3, 12> points{};
    for (std::size_t index = 0; index < unitPoints.size(); ++index) {
        const Vec3 point = unitPoints[index];
        points[index] = {center.x + point.x * radius, center.y + point.y * radius, center.z + point.z * radius};
    }

    return points;
}

constexpr std::array<std::array<int, 3>, 20> icosahedronFaces()
{
    return {
        std::array<int, 3>{0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 1},
        {1, 6, 2}, {2, 6, 7}, {2, 7, 3}, {3, 7, 8}, {3, 8, 4},
        {4, 8, 9}, {4, 9, 5}, {5, 9, 10}, {5, 10, 1}, {1, 10, 6},
        {11, 7, 6}, {11, 8, 7}, {11, 9, 8}, {11, 10, 9}, {11, 6, 10},
    };
}

void appendIcosahedronMarker(std::vector<LineVertex>& vertices, Vec3 center, Vec3 color, float radius)
{
    const std::array<Vec3, 12> points = buildIcosahedronPoints(center, radius);

    constexpr std::array<std::array<int, 2>, 30> edges{
        std::array<int, 2>{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5},
        {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 1},
        {1, 6}, {2, 6}, {2, 7}, {3, 7}, {3, 8},
        {4, 8}, {4, 9}, {5, 9}, {5, 10}, {1, 10},
        {6, 7}, {7, 8}, {8, 9}, {9, 10}, {10, 6},
        {11, 6}, {11, 7}, {11, 8}, {11, 9}, {11, 10},
    };

    for (const auto& edge : edges) {
        addLine(vertices, points[edge[0]], points[edge[1]], color);
    }
}

void appendIcosahedronFill(std::vector<ColoredVertex>& vertices, Vec3 center, Vec3 color, float radius)
{
    const std::array<Vec3, 12> points = buildIcosahedronPoints(center, radius);
    const ColorRgba fillColor{color.x, color.y, color.z, 1.0f};

    for (const auto& face : icosahedronFaces()) {
        addTriangle(vertices, points[face[0]], points[face[1]], points[face[2]], fillColor);
    }
}

void appendRayParticle(std::vector<ColoredVertex>& fillVertices, const RenderRayParticle& particle)
{
    const float relativeEnergy = particle.initialEnergy > 0.0f ? clamp01(particle.energy / particle.initialEnergy) : 0.0f;
    if (relativeEnergy <= 0.01f) {
        return;
    }

    const Vec3 coreYellow{1.0f, 0.92f, 0.16f};
    const Vec3 haloYellow{1.0f, 0.70f, 0.04f};
    const float visibleEnergy = relativeEnergy * relativeEnergy;

    const float coreRadius = particle.radius * (0.25f + relativeEnergy * 0.95f);
    const float innerHaloRadius = particle.radius * (1.35f + (1.0f - relativeEnergy) * 0.75f);
    const float outerHaloRadius = particle.radius * (2.35f + (1.0f - relativeEnergy) * 1.25f);

    const ColorRgba coreColor{coreYellow.x, coreYellow.y, coreYellow.z, 0.85f * std::pow(relativeEnergy, 1.5f)};
    const ColorRgba innerHaloColor{haloYellow.x, haloYellow.y, haloYellow.z, 0.30f * relativeEnergy};
    const ColorRgba outerHaloColor{haloYellow.x, haloYellow.y, haloYellow.z, 0.12f * visibleEnergy};

    const std::array<Vec3, 12> outerHaloPoints = buildIcosahedronPoints(particle.position, outerHaloRadius);
    for (const auto& face : icosahedronFaces()) {
        addTriangle(fillVertices, outerHaloPoints[face[0]], outerHaloPoints[face[1]], outerHaloPoints[face[2]], outerHaloColor);
    }

    const std::array<Vec3, 12> innerHaloPoints = buildIcosahedronPoints(particle.position, innerHaloRadius);
    for (const auto& face : icosahedronFaces()) {
        addTriangle(fillVertices, innerHaloPoints[face[0]], innerHaloPoints[face[1]], innerHaloPoints[face[2]], innerHaloColor);
    }

    const std::array<Vec3, 12> points = buildIcosahedronPoints(particle.position, coreRadius);

    for (const auto& face : icosahedronFaces()) {
        addTriangle(fillVertices, points[face[0]], points[face[1]], points[face[2]], coreColor);
    }
}

void appendSelectableIcosahedronMarker(std::vector<ColoredVertex>& opaqueFillVertices, std::vector<LineVertex>& lineVertices, Vec3 position, Vec3 color, bool selected)
{
    const Vec3 fillColor = selected ? selectedMarkerColor(color) : color;
    const Vec3 edgeColor = darker(fillColor);

    appendIcosahedronFill(opaqueFillVertices, position, fillColor, 0.13f);
    appendIcosahedronMarker(lineVertices, position, edgeColor, 0.13f);
}

void appendSourceMarker(std::vector<ColoredVertex>& opaqueFillVertices, std::vector<LineVertex>& lineVertices, const GuiSource& source, bool selected)
{
    if (!source.visible) {
        return;
    }

    appendSelectableIcosahedronMarker(opaqueFillVertices, lineVertices, source.position, source.color, selected);
}

void appendReceiverMarker(std::vector<ColoredVertex>& opaqueFillVertices, std::vector<LineVertex>& lineVertices, const GuiReceiver& receiver, bool selected)
{
    if (!receiver.visible) {
        return;
    }

    appendSelectableIcosahedronMarker(opaqueFillVertices, lineVertices, receiver.position, receiver.color, selected);
}

void appendDraftLines(std::vector<LineVertex>& vertices, const GuiPlaneDraft& draft)
{
    if (!draft.active) {
        return;
    }

    const Vec3 lineColor = draft.isEditingExistingPlane() ? Vec3{0.65f, 0.42f, 1.0f} : Vec3{0.25f, 0.95f, 0.72f};
    const Vec3 closingColor{0.95f, 0.74f, 0.25f};
    const Vec3 cursorColor{1.0f, 0.22f, 0.22f};

    for (std::size_t index = 1; index < draft.points.size(); ++index) {
        addLine(vertices, draft.points[index - 1], draft.points[index], lineColor);
    }

    if (draft.points.size() >= 3) {
        addLine(vertices, draft.points.back(), draft.points.front(), closingColor);
    }

    for (const Vec3& point : draft.points) {
        appendPointMarker(vertices, point, lineColor);
    }

    appendPointMarker(vertices, draft.cursor, cursorColor);
    if (!draft.points.empty()) {
        addLine(vertices, draft.points.back(), draft.cursor, cursorColor);
    }
}

}  // namespace

std::vector<LineVertex> GuiScenarioRenderMapper::buildLineVertices(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft) const
{
    return buildRenderScene(scenario, selection, draft).lineVertices;
}

RenderScene GuiScenarioRenderMapper::buildRenderScene(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft) const
{
    return buildRenderScene(scenario, selection, draft, {});
}

RenderScene GuiScenarioRenderMapper::buildRenderScene(const GuiScenario& scenario, const GuiSelection& selection, const GuiPlaneDraft& draft, const RenderSimulationOverlay& simulationOverlay) const
{
    RenderScene scene;
    scene.fillVertices.reserve(scenario.planes.size() * 6);
    scene.opaqueFillVertices.reserve((scenario.sources.size() + scenario.receivers.size()) * 60);
    scene.lineVertices.reserve(scenario.planes.size() * 12 + scenario.sources.size() * 60 + scenario.receivers.size() * 60 + draft.points.size() * 8 + 32);
    const TriangleEnergyLookup triangleEnergyLookup = buildTriangleEnergyLookup(simulationOverlay);

    for (const GuiPlane& plane : scenario.planes) {
        const bool selected = selection.isPlaneSelected() && plane.id == selection.selectedPlaneId();
        appendPlaneFill(scene.fillVertices, plane, simulationOverlay, triangleEnergyLookup);
        appendPlaneLines(scene.lineVertices, plane, selected);
        if (selected) {
            appendTriangleInspection(scene.fillVertices, scene.lineVertices, plane, selection.selectedTriangleId());
        }
    }

    for (const GuiSource& source : scenario.sources) {
        appendSourceMarker(scene.opaqueFillVertices, scene.lineVertices, source, selection.isSourceSelected() && source.id == selection.selectedSourceId());
    }

    for (const GuiReceiver& receiver : scenario.receivers) {
        appendReceiverMarker(scene.opaqueFillVertices, scene.lineVertices, receiver, selection.isReceiverSelected() && receiver.id == selection.selectedReceiverId());
    }

    if (simulationOverlay.active && simulationOverlay.showRayTracing) {
        for (const RenderRayParticle& particle : simulationOverlay.rayParticles) {
            appendRayParticle(scene.fillVertices, particle);
        }
    }

    appendDraftLines(scene.lineVertices, draft);
    AxisGizmo::appendLines(scene.lineVertices);
    return scene;
}

}  // namespace gui
