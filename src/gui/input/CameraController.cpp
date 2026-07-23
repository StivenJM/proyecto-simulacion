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

Vec3 add(Vec3 a, Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 scale(Vec3 value, float factor)
{
    return {value.x * factor, value.y * factor, value.z * factor};
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

void CameraController::update(GLFWwindow* window, float deltaTime, bool mouseLookEnabled)
{
    if (window == nullptr) {
        return;
    }

    if (!mouseLookEnabled) {
        resetMouseLook();
        const float orbitSpeed = 70.0f * std::max(deltaTime, 0.0f);
        float yawDelta = 0.0f;
        float pitchDelta = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            yawDelta -= orbitSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            yawDelta += orbitSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            pitchDelta += orbitSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            pitchDelta -= orbitSpeed;
        }

        if (yawDelta == 0.0f && pitchDelta == 0.0f) {
            return;
        }

        const Vec3 target{0.0f, 0.0f, 0.0f};
        const Vec3 offset = subtract(position_, target);
        const float distance = std::max(std::sqrt(dot(offset, offset)), 0.001f);
        float orbitYaw = std::atan2(offset.x, offset.z) * 180.0f / pi + yawDelta;
        float orbitPitch = std::asin(std::clamp(offset.y / distance, -1.0f, 1.0f)) * 180.0f / pi + pitchDelta;
        orbitPitch = std::clamp(orbitPitch, -80.0f, 80.0f);

        const float yaw = radians(orbitYaw);
        const float pitch = radians(orbitPitch);
        position_ = {
            target.x + distance * std::cos(pitch) * std::sin(yaw),
            target.y + distance * std::sin(pitch),
            target.z + distance * std::cos(pitch) * std::cos(yaw),
        };
        front_ = normalize(subtract(target, position_));
        updateAnglesFromFront();
        return;
    }

    const float cameraSpeed = moveSpeed_ * std::max(deltaTime, 0.0f);
    const Vec3 right = normalize(cross(front_, up_));

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        position_ = add(position_, scale(front_, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        position_ = subtract(position_, scale(front_, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        position_ = subtract(position_, scale(right, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        position_ = add(position_, scale(right, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position_ = add(position_, scale(up_, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position_ = subtract(position_, scale(up_, cameraSpeed));
    }

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    if (!mouseLookActive_ || firstMouseSample_) {
        lastMouseX_ = mouseX;
        lastMouseY_ = mouseY;
        mouseLookActive_ = true;
        firstMouseSample_ = false;
        return;
    }

    const float xOffset = static_cast<float>(mouseX - lastMouseX_) * mouseSensitivity_;
    const float yOffset = static_cast<float>(lastMouseY_ - mouseY) * mouseSensitivity_;
    lastMouseX_ = mouseX;
    lastMouseY_ = mouseY;

    yaw_ += xOffset;
    pitch_ = std::clamp(pitch_ + yOffset, -89.0f, 89.0f);
    updateFrontFromAngles();
}

void CameraController::resetMouseLook()
{
    mouseLookActive_ = false;
    firstMouseSample_ = true;
}

Vec3 CameraController::position() const
{
    return position_;
}

Mat4 CameraController::viewProjectionMatrix() const
{
    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    return multiply(perspective(radians(fovDegrees_), aspect, 0.1f, 100.0f), lookAt(position_, add(position_, front_), up_));
}

Mat4 CameraController::viewProjectionFrom(Vec3 eye, Vec3 target, float fovDegrees) const
{
    const float aspect = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    return multiply(perspective(radians(fovDegrees), aspect, 0.05f, 100.0f), lookAt(eye, target, {0.0f, 1.0f, 0.0f}));
}

void CameraController::setPose(Vec3 position, Vec3 target)
{
    position_ = position;
    front_ = normalize(subtract(target, position));
    if (dot(front_, front_) <= 0.0001f) {
        front_ = {0.0f, 0.0f, -1.0f};
    }
    updateAnglesFromFront();
    firstMouseSample_ = true;
}

void CameraController::updateFrontFromAngles()
{
    const float yaw = radians(yaw_);
    const float pitch = radians(pitch_);
    front_ = normalize({
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch),
    });
}

void CameraController::updateAnglesFromFront()
{
    const Vec3 normalizedFront = normalize(front_);
    pitch_ = std::asin(std::clamp(normalizedFront.y, -1.0f, 1.0f)) * 180.0f / pi;
    yaw_ = std::atan2(normalizedFront.z, normalizedFront.x) * 180.0f / pi;
    pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
}

}  // namespace gui
