#include "CameraController.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>

namespace gui {
namespace {

constexpr float pi = 3.14159265358979323846f;

float radians(float degrees)
{
    return degrees * pi / 180.0f;
}

Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 normalize(Vec3 value)
{
    const float length = std::sqrt(dot(value, value));
    if (length <= 0.0001f) {
        return {0.0f, 0.0f, 0.0f};
    }

    return {value.x / length, value.y / length, value.z / length};
}

Mat4 multiply(const Mat4& a, const Mat4& b)
{
    Mat4 result;
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int index = 0; index < 4; ++index) {
                sum += a.values[index * 4 + row] * b.values[column * 4 + index];
            }
            result.values[column * 4 + row] = sum;
        }
    }

    return result;
}

Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    const float tanHalfFov = std::tan(fovRadians / 2.0f);

    Mat4 result;
    result.values[0] = 1.0f / (aspect * tanHalfFov);
    result.values[5] = 1.0f / tanHalfFov;
    result.values[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.values[11] = -1.0f;
    result.values[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    return result;
}

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    const Vec3 forward = normalize(subtract(center, eye));
    const Vec3 side = normalize(cross(forward, up));
    const Vec3 cameraUp = cross(side, forward);

    Mat4 result;
    result.values[0] = side.x;
    result.values[4] = side.y;
    result.values[8] = side.z;

    result.values[1] = cameraUp.x;
    result.values[5] = cameraUp.y;
    result.values[9] = cameraUp.z;

    result.values[2] = -forward.x;
    result.values[6] = -forward.y;
    result.values[10] = -forward.z;

    result.values[12] = -dot(side, eye);
    result.values[13] = -dot(cameraUp, eye);
    result.values[14] = dot(forward, eye);
    result.values[15] = 1.0f;

    return result;
}

}  // namespace

void CameraController::setViewport(int width, int height)
{
    viewportWidth_ = std::max(width, 1);
    viewportHeight_ = std::max(height, 1);
}

void CameraController::update(GLFWwindow* window, float deltaTime)
{
    const float orbitSpeed = 70.0f * deltaTime;
    const float zoomSpeed = 4.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        yaw_ -= orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        yaw_ += orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        pitch_ += orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        pitch_ -= orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        distance_ += zoomSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        distance_ -= zoomSpeed;
    }

    pitch_ = std::clamp(pitch_, -80.0f, 80.0f);
    distance_ = std::clamp(distance_, 2.5f, 14.0f);
}

Vec3 CameraController::position() const
{
    const float yaw = radians(yaw_);
    const float pitch = radians(pitch_);
    return {
        target_.x + distance_ * std::cos(pitch) * std::sin(yaw),
        target_.y + distance_ * std::sin(pitch),
        target_.z + distance_ * std::cos(pitch) * std::cos(yaw),
    };
}

Mat4 CameraController::viewProjectionMatrix() const
{
    const Vec3 eye = position();

    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    return multiply(perspective(radians(45.0f), aspect, 0.1f, 100.0f), lookAt(eye, target_, {0.0f, 1.0f, 0.0f}));
}

Mat4 CameraController::viewProjectionFrom(Vec3 eye, Vec3 target, float fovDegrees) const
{
    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    return multiply(perspective(radians(fovDegrees), aspect, 0.05f, 100.0f), lookAt(eye, target, {0.0f, 1.0f, 0.0f}));
}

}  // namespace gui
