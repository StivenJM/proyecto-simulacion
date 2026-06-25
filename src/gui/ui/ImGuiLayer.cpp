#include "ImGuiLayer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

namespace gui {

ImGuiLayer::~ImGuiLayer()
{
    shutdown();
}

bool ImGuiLayer::initialize(GLFWwindow* window)
{
    if (initialized_) {
        return true;
    }

    if (window == nullptr) {
        std::cerr << "Cannot initialize ImGui without a GLFW window.\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.09f, 0.11f, 0.96f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.32f, 0.52f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.42f, 0.68f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.28f, 0.46f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.22f, 0.27f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.25f, 0.31f, 1.0f);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend.\n";
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend.\n";
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    initialized_ = true;
    return true;
}

void ImGuiLayer::beginFrame()
{
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render()
{
    if (!initialized_) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::shutdown()
{
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

}  // namespace gui
