#pragma once

#include <unordered_map>

struct GLFWwindow;

namespace gui {

class InputState {
public:
    void update(GLFWwindow* window);
    bool isDown(int key) const;
    bool wasPressed(int key) const;

private:
    GLFWwindow* window_ = nullptr;
    mutable std::unordered_map<int, bool> previousKeyStates_;
};

}  // namespace gui
