#pragma once

#include "gui/math/MathTypes.h"

struct GLFWwindow;

namespace gui {

class CameraController {
public:
    void setViewport(int width, int height);
    void update(GLFWwindow* window, float deltaTime, bool mouseLookEnabled);
    void resetMouseLook();
    void setPose(Vec3 position, Vec3 target);
    Vec3 position() const;
    Mat4 viewProjectionMatrix() const;
    Mat4 viewProjectionFrom(Vec3 eye, Vec3 target, float fovDegrees = 60.0f) const;

private:
    void updateFrontFromAngles();
    void updateAnglesFromFront();

    int viewportWidth_ = 1280;
    int viewportHeight_ = 720;
    float yaw_ = -45.0f;
    float pitch_ = -20.0f;
    float fovDegrees_ = 45.0f;
    float moveSpeed_ = 4.0f;
    float mouseSensitivity_ = 0.10f;
    Vec3 position_{4.0f, 2.4f, 4.0f};
    Vec3 front_{-0.66f, -0.34f, -0.66f};
    Vec3 up_{0.0f, 1.0f, 0.0f};
    bool mouseLookActive_ = false;
    bool firstMouseSample_ = true;
    double lastMouseX_ = 0.0;
    double lastMouseY_ = 0.0;
};

}  // namespace gui
