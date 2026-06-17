#pragma once

#include "AppMode.h"
#include "CameraController.h"

namespace gui {

class OpenGLRenderer {
public:
    OpenGLRenderer() = default;
    ~OpenGLRenderer();

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool initialize();
    void render(const Mat4& viewProjectionMatrix, AppMode mode, bool simulationStarted);
    void shutdown();

private:
    unsigned int shaderProgram_ = 0;
    unsigned int vertexArray_ = 0;
    unsigned int vertexBuffer_ = 0;
    int lineVertexCount_ = 0;
};

}  // namespace gui
