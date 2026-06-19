#pragma once

#include "gui/AppMode.h"
#include "gui/input/CameraController.h"
#include "gui/rendering/RenderTypes.h"

#include <vector>

namespace gui {

class OpenGLRenderer {
public:
    OpenGLRenderer() = default;
    ~OpenGLRenderer();

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool initialize();
    void render(const Mat4& viewProjectionMatrix, const std::vector<LineVertex>& lineVertices, AppMode mode, bool simulationStarted);
    void shutdown();

private:
    unsigned int shaderProgram_ = 0;
    unsigned int vertexArray_ = 0;
    unsigned int vertexBuffer_ = 0;
};

}  // namespace gui
