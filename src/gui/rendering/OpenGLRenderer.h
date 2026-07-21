#pragma once

#include "gui/AppMode.h"
#include "gui/input/CameraController.h"
#include "gui/rendering/RenderTypes.h"

#include <vector>

struct GLUquadric;

namespace gui {

class OpenGLRenderer {
public:
    OpenGLRenderer() = default;
    ~OpenGLRenderer();

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool initialize();
    void render(const Mat4& viewProjectionMatrix, const RenderScene& scene, AppMode mode, bool simulationStarted);
    void shutdown();

private:
    unsigned int shaderProgram_ = 0;
    unsigned int lineVertexArray_ = 0;
    unsigned int lineVertexBuffer_ = 0;
    unsigned int fillVertexArray_ = 0;
    unsigned int fillVertexBuffer_ = 0;
    GLUquadric* rayParticleQuadric_ = nullptr;
};

}  // namespace gui
