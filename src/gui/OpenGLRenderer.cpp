#include "OpenGLRenderer.h"

#include <glad/glad.h>

#include <array>
#include <iostream>

namespace gui {
namespace {

struct Vertex {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

constexpr const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

uniform mat4 uViewProjection;

out vec3 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
)glsl";

constexpr const char* fragmentShaderSource = R"glsl(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vColor, 1.0);
}
)glsl";

void addLine(std::array<Vertex, 30>& vertices, int& index, Vec3 from, Vec3 to, Vec3 color)
{
    vertices[index++] = {from.x, from.y, from.z, color.x, color.y, color.z};
    vertices[index++] = {to.x, to.y, to.z, color.x, color.y, color.z};
}

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

    std::array<Vertex, 30> vertices{};
    int index = 0;

    const Vec3 roomColor{0.72f, 0.78f, 0.86f};
    const Vec3 floorA{-2.0f, -1.0f, -1.5f};
    const Vec3 floorB{2.0f, -1.0f, -1.5f};
    const Vec3 floorC{2.0f, -1.0f, 1.5f};
    const Vec3 floorD{-2.0f, -1.0f, 1.5f};
    const Vec3 roofA{-2.0f, 1.0f, -1.5f};
    const Vec3 roofB{2.0f, 1.0f, -1.5f};
    const Vec3 roofC{2.0f, 1.0f, 1.5f};
    const Vec3 roofD{-2.0f, 1.0f, 1.5f};

    addLine(vertices, index, floorA, floorB, roomColor);
    addLine(vertices, index, floorB, floorC, roomColor);
    addLine(vertices, index, floorC, floorD, roomColor);
    addLine(vertices, index, floorD, floorA, roomColor);
    addLine(vertices, index, roofA, roofB, roomColor);
    addLine(vertices, index, roofB, roofC, roomColor);
    addLine(vertices, index, roofC, roofD, roomColor);
    addLine(vertices, index, roofD, roofA, roomColor);
    addLine(vertices, index, floorA, roofA, roomColor);
    addLine(vertices, index, floorB, roofB, roomColor);
    addLine(vertices, index, floorC, roofC, roomColor);
    addLine(vertices, index, floorD, roofD, roomColor);

    addLine(vertices, index, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.25f, 0.25f});
    addLine(vertices, index, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.25f, 1.0f, 0.25f});
    addLine(vertices, index, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.25f, 0.45f, 1.0f});

    lineVertexCount_ = index;

    glGenVertexArrays(1, &vertexArray_);
    glGenBuffers(1, &vertexBuffer_);

    glBindVertexArray(vertexArray_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, lineVertexCount_ * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    return true;
}

void OpenGLRenderer::render(const Mat4& viewProjectionMatrix, AppMode mode, bool simulationStarted)
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

    glBindVertexArray(vertexArray_);
    glDrawArrays(GL_LINES, 0, lineVertexCount_);
    glBindVertexArray(0);
}

void OpenGLRenderer::shutdown()
{
    if (vertexBuffer_ != 0) {
        glDeleteBuffers(1, &vertexBuffer_);
        vertexBuffer_ = 0;
    }
    if (vertexArray_ != 0) {
        glDeleteVertexArrays(1, &vertexArray_);
        vertexArray_ = 0;
    }
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
}

}  // namespace gui
