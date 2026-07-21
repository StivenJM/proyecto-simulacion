#pragma once

#include "gui/math/MathTypes.h"

#include <array>
#include <vector>

namespace gui {

struct LineVertex {
    Vec3 position;
    Vec3 color;
};

struct ColorRgba {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct ColoredVertex {
    Vec3 position;
    ColorRgba color;
};

struct RenderPlaneEnergy {
    int planeId = 0;
    float energy = 0.0f;
};

struct RenderTriangleEnergy {
    int planeId = 0;
    int triangleId = 0;
    float energy = 0.0f;
    std::array<Vec3, 3> vertices{};
    bool hasGeometry = false;
};

struct RenderRayParticle {
    Vec3 position{0.0f, 0.0f, 0.0f};
    float radius = 0.04f;
    float energy = 1.0f;
    float initialEnergy = 1.0f;
};

struct RenderRayTrail {
    Vec3 start{0.0f, 0.0f, 0.0f};
    Vec3 end{0.0f, 0.0f, 0.0f};
    float energy = 1.0f;
};

struct RenderSimulationOverlay {
    bool active = false;
    bool showDiffuseEnergy = true;
    bool showRayTracing = true;
    std::vector<RenderPlaneEnergy> planeEnergy;
    std::vector<RenderTriangleEnergy> triangleEnergy;
    std::vector<RenderRayParticle> rayParticles;
    std::vector<RenderRayTrail> rayTrails;
};

struct RenderScene {
    std::vector<ColoredVertex> opaqueFillVertices;
    std::vector<ColoredVertex> fillVertices;
    std::vector<LineVertex> lineVertices;
    std::vector<LineVertex> alwaysVisibleLineVertices;
    std::vector<RenderRayParticle> rayParticles;
};

}  // namespace gui
