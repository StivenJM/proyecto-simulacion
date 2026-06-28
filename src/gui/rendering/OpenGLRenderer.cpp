#include "OpenGLRenderer.h"

#include <glad/glad.h>

#include <iostream>

namespace gui {
namespace {

constexpr const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;

uniform mat4 uViewProjection;

out vec4 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
)glsl";

constexpr const char* fragmentShaderSource = R"glsl(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vColor;
}
)glsl";

unsigned int compileShader(unsigned int type, const char* source)
{
    const unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

unsigned int createShaderProgram()
{
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (vertexShader == 0) {
        return 0;
    }

    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << '\n';
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

}  // namespace

OpenGLRenderer::~OpenGLRenderer()
{
    shutdown();
}

bool OpenGLRenderer::initialize()
{
    shaderProgram_ = createShaderProgram();
    if (shaderProgram_ == 0) {
        return false;
    }

    glGenVertexArrays(1, &lineVertexArray_);
    glGenBuffers(1, &lineVertexBuffer_);
    glGenVertexArrays(1, &fillVertexArray_);
    glGenBuffers(1, &fillVertexBuffer_);

    glBindVertexArray(lineVertexArray_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), reinterpret_cast<void*>(sizeof(Vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(fillVertexArray_);
    glBindBuffer(GL_ARRAY_BUFFER, fillVertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(sizeof(Vec3)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    return true;
}

void OpenGLRenderer::render(const Mat4& viewProjectionMatrix, const RenderScene& scene, AppMode mode, bool simulationStarted)
{
    if (mode == AppMode::Preparation) {
        glClearColor(0.055f, 0.075f, 0.11f, 1.0f);
    } else if (simulationStarted) {
        glClearColor(0.12f, 0.075f, 0.035f, 1.0f);
    } else {
        glClearColor(0.09f, 0.06f, 0.10f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram_);
    const int matrixLocation = glGetUniformLocation(shaderProgram_, "uViewProjection");
    glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, viewProjectionMatrix.values.data());

    if (!scene.fillVertices.empty()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glBindVertexArray(fillVertexArray_);
        glBindBuffer(GL_ARRAY_BUFFER, fillVertexBuffer_);
        glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(scene.fillVertices.size() * sizeof(ColoredVertex)), scene.fillVertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(scene.fillVertices.size()));

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    glBindVertexArray(lineVertexArray_);
    glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(scene.lineVertices.size() * sizeof(LineVertex)), scene.lineVertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<int>(scene.lineVertices.size()));
    glBindVertexArray(0);
}

void OpenGLRenderer::shutdown()
{
    if (fillVertexBuffer_ != 0) {
        glDeleteBuffers(1, &fillVertexBuffer_);
        fillVertexBuffer_ = 0;
    }
    if (fillVertexArray_ != 0) {
        glDeleteVertexArrays(1, &fillVertexArray_);
        fillVertexArray_ = 0;
    }
    if (lineVertexBuffer_ != 0) {
        glDeleteBuffers(1, &lineVertexBuffer_);
        lineVertexBuffer_ = 0;
    }
    if (lineVertexArray_ != 0) {
        glDeleteVertexArrays(1, &lineVertexArray_);
        lineVertexArray_ = 0;
    }
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
}

}  // namespace gui
