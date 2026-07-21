#pragma once

#include "gui/math/MathTypes.h"

struct GLFWwindow;

namespace gui {

class CameraController {
public:
    void setViewport(int width, int height);
    void update(GLFWwindow* window, float deltaTime);
    Vec3 position() const;
    Mat4 viewProjectionMatrix() const;
    Mat4 viewProjectionFrom(Vec3 eye, Vec3 target, float fovDegrees = 60.0f) const;

private:
    int viewportWidth_ = 1280;
    int viewportHeight_ = 720;
    float yaw_ = 45.0f;
    float pitch_ = 28.0f;
    float distance_ = 7.0f;
    Vec3 target_{0.0f, 0.0f, 0.0f};
};

}  // namespace gui
