#pragma once

struct GLFWwindow;

namespace gui {

class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    bool initialize(GLFWwindow* window);
    void beginFrame();
    void render();
    void shutdown();

private:
    bool initialized_ = false;
};

}  // namespace gui
