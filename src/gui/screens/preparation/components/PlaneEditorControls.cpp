#include "PlaneEditorControls.h"

#include <GLFW/glfw3.h>

namespace gui {

bool PlaneEditorControls::addPlanePressed(const InputState& input) { return input.wasPressed(GLFW_KEY_P); }
bool PlaneEditorControls::addRoomPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_B); }
bool PlaneEditorControls::selectNextPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_C); }
bool PlaneEditorControls::newDraftPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_N); }
bool PlaneEditorControls::addDraftPointPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_M); }
bool PlaneEditorControls::finalizeDraftPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_F); }
bool PlaneEditorControls::editSelectedPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_V); }
bool PlaneEditorControls::cancelDraftPressed(const InputState& input) { return input.wasPressed(GLFW_KEY_X); }

Vec3 PlaneEditorControls::movementDelta(const InputState& input, float deltaTime)
{
    const float moveSpeed = 2.0f * deltaTime;
    Vec3 delta{0.0f, 0.0f, 0.0f};

    if (input.isDown(GLFW_KEY_J)) delta.x -= moveSpeed;
    if (input.isDown(GLFW_KEY_L)) delta.x += moveSpeed;
    if (input.isDown(GLFW_KEY_U)) delta.y += moveSpeed;
    if (input.isDown(GLFW_KEY_O)) delta.y -= moveSpeed;
    if (input.isDown(GLFW_KEY_I)) delta.z -= moveSpeed;
    if (input.isDown(GLFW_KEY_K)) delta.z += moveSpeed;

    return delta;
}

}  // namespace gui
