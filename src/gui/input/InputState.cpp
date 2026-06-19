#include "InputState.h"

#include <GLFW/glfw3.h>

namespace gui {

void InputState::update(GLFWwindow* window)
{
    window_ = window;
}

bool InputState::isDown(int key) const
{
    return window_ != nullptr && glfwGetKey(window_, key) == GLFW_PRESS;
}

bool InputState::wasPressed(int key) const
{
    const bool isPressed = isDown(key);
    const bool wasPressedBefore = previousKeyStates_[key];
    previousKeyStates_[key] = isPressed;
    return isPressed && !wasPressedBefore;
}

}  // namespace gui
