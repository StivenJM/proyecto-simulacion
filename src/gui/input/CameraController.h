#pragma once

#include <array>

struct GLFWwindow;

namespace gui {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Mat4 {
    std::array<float, 16> values{};
};

class CameraController {
public:
    void setViewport(int width, int height);
    void update(GLFWwindow* window, float deltaTime);
    Mat4 viewProjectionMatrix() const;

private:
    int viewportWidth_ = 1280;
    int viewportHeight_ = 720;
    float yaw_ = 45.0f;
    float pitch_ = 28.0f;
    float distance_ = 7.0f;
    Vec3 target_{0.0f, 0.0f, 0.0f};
};

}  // namespace gui
