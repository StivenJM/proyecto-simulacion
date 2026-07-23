#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include "resource.h"
#endif

#include "composition/GuiComposition.h"

#include <iostream>
#include <string>

#if defined(_WIN32)
namespace {

void applyWindowIcon(GLFWwindow* window)
{
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    const auto largeIcon = static_cast<HICON>(LoadImageW(
        instance,
        MAKEINTRESOURCEW(IDI_APP_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXICON),
        GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR | LR_SHARED
    ));
    const auto smallIcon = static_cast<HICON>(LoadImageW(
        instance,
        MAKEINTRESOURCEW(IDI_APP_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR | LR_SHARED
    ));

    const HWND hwnd = glfwGetWin32Window(window);
    if (hwnd != nullptr) {
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(largeIcon));
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon));
    }
}

}  // namespace
#endif

namespace gui {

App::App()
    : services_(GuiComposition::createServices({})),
      scenario_(services_.scenarioService->createInitialScenario(selection_)),
      preparationScreen_(scenario_, selection_, *services_.planeService, *services_.sourceService, *services_.receiverService),
      simulationScreen_(*services_.simulationService)
{
}

App::~App()
{
    imguiLayer_.shutdown();
    renderer_.shutdown();

    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    glfwTerminate();
}

bool App::initialize()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window_ = glfwCreateWindow(1280, 720, "Acoustic Simulator", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Failed to create the GLFW window.\n";
        return false;
    }

#if defined(_WIN32)
    applyWindowIcon(window_);
#endif

    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, &App::onFramebufferResize);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD.\n";
        return false;
    }

    int framebufferWidth = 1280;
    int framebufferHeight = 720;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    resize(framebufferWidth, framebufferHeight);
    updateWindowTitle();

    if (!renderer_.initialize()) {
        return false;
    }

    return imguiLayer_.initialize(window_);
}

void App::run()
{
    float previousTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window_)) {
        const float currentTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        input_.update(window_);
        const bool imguiWantsKeyboard = ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard;
        bool simulationStarted = processInput(imguiWantsKeyboard);

        imguiLayer_.beginFrame();
        renderCommandsModal();
        if (mode_ == AppMode::Preparation) {
            sceneHierarchyPanel_.render(preparationScreen_);
            planeEditorPanel_.render(preparationScreen_);
            updateWindowTitle();
        } else {
            simulationScreen_.update(deltaTime, scenario_);
            simulationStarted = simulationScreen_.renderPanel(scenario_) || simulationStarted;
            syncSimulationCameraViewMode();
            updateWindowTitle();
        }
        renderInteractionModeHud();

        if (simulationStarted) {
            previousTime = static_cast<float>(glfwGetTime());
        }

        if (interactionMode_ == InteractionMode::Ui && !imguiWantsKeyboard && mode_ == AppMode::Preparation && preparationScreen_.handleInput(input_, deltaTime)) {
            updateWindowTitle();
        }

        if ((interactionMode_ == InteractionMode::Camera) || (!imguiWantsKeyboard && interactionMode_ == InteractionMode::Ui)) {
            camera_.update(window_, deltaTime, interactionMode_ == InteractionMode::Camera);
        }

        renderer_.render(
            simulationViewProjection(),
            renderMapper_.buildRenderScene(scenario_, selection_, preparationScreen_.draft(), simulationScreen_.renderOverlay(), simulationCameraPosition()),
            mode_,
            simulationScreen_.isStarted()
        );
        imguiLayer_.render();

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

void App::onFramebufferResize(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) {
        app->resize(width, height);
    }
}

void App::resize(int width, int height)
{
    glViewport(0, 0, width, height);
    camera_.setViewport(width, height);
}

bool App::processInput(bool imguiWantsKeyboard)
{
    if (input_.isDown(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window_, true);
    }

    if (input_.wasPressed(GLFW_KEY_F1)) {
        showCommandsModal_ = !showCommandsModal_;
        if (showCommandsModal_) {
            setInteractionMode(InteractionMode::Ui);
        }
    }

    if (input_.wasPressed(GLFW_KEY_F2)) {
        toggleInteractionMode();
    }

    if (imguiWantsKeyboard && interactionMode_ == InteractionMode::Ui) {
        return false;
    }

    if (input_.wasPressed(GLFW_KEY_TAB)) {
        toggleMode();
    }

    if (input_.wasPressed(GLFW_KEY_ENTER)) {
        return startSimulation();
    }

    return false;
}

void App::toggleMode()
{
    if (mode_ == AppMode::Preparation) {
        mode_ = AppMode::Simulation;
        lastSimulationViewMode_ = SimulationViewMode::External;
        simulationScreen_.beginPrecompute(scenario_);
    } else {
        mode_ = AppMode::Preparation;
        simulationScreen_.reset();
    }

    updateWindowTitle();
}

bool App::startSimulation()
{
    if (mode_ != AppMode::Simulation) {
        std::cout << "Switch to simulation mode before starting the simulation.\n";
        return false;
    }

    if (simulationScreen_.start(scenario_)) {
        updateWindowTitle();
        std::cout << "Simulation started. Preparation editing is locked.\n";
        return true;
    }

    return false;
}

Mat4 App::simulationViewProjection() const
{
    return camera_.viewProjectionMatrix();
}

void App::toggleInteractionMode()
{
    setInteractionMode(interactionMode_ == InteractionMode::Ui ? InteractionMode::Camera : InteractionMode::Ui);
}

void App::setInteractionMode(InteractionMode mode)
{
    if (interactionMode_ == mode) {
        return;
    }

    interactionMode_ = mode;
    camera_.resetMouseLook();
    if (window_ != nullptr) {
        glfwSetInputMode(window_, GLFW_CURSOR, interactionMode_ == InteractionMode::Camera ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void App::renderInteractionModeHud()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 topCenter{viewport->WorkPos.x + viewport->WorkSize.x * 0.5f, viewport->WorkPos.y + 12.0f};
    ImGui::SetNextWindowPos(topCenter, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::Begin("InteractionModeHud", nullptr, flags);
    const bool cameraMode = interactionMode_ == InteractionMode::Camera;
    ImGui::TextColored(cameraMode ? ImVec4(0.35f, 0.75f, 1.0f, 1.0f) : ImVec4(0.45f, 1.0f, 0.55f, 1.0f),
        "Mode: %s", cameraMode ? "CAMERA" : "UI");
    ImGui::TextDisabled("F2 toggle · F1 commands");
    ImGui::End();
}

void App::renderCommandsModal()
{
    if (showCommandsModal_) {
        ImGui::OpenPopup("Commands");
    }

    bool modalOpen = showCommandsModal_;
    if (ImGui::BeginPopupModal("Commands", &modalOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Interaction");
        ImGui::Separator();
        if (ImGui::BeginTable("InteractionCommands", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Key");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();
            auto row = [](const char* key, const char* action) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(action);
            };
            row("F1", "Open/close this commands window");
            row("F2", "Toggle UI mode / Camera mode");
            row("Tab", "Switch Preparation / Simulation");
            row("Enter", "Start or restart simulation when ready");
            row("Esc", "Close the application");
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("UI mode");
        ImGui::Separator();
        if (ImGui::BeginTable("UiModeCommands", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Key / Input");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();
            auto row = [](const char* key, const char* action) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(action);
            };
            row("Mouse", "Interact with ImGui panels");
            row("Arrow keys", "Orbit/rotate the scene camera");
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Camera mode");
        ImGui::Separator();
        if (ImGui::BeginTable("CameraCommands", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Key / Input");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();
            auto row = [](const char* key, const char* action) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(action);
            };
            row("Mouse movement", "Rotate camera view");
            row("Arrow Up / Down", "Move forward / backward");
            row("Arrow Left / Right", "Strafe left / right");
            row("Q / E", "Move up / down");
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Preparation shortcuts");
        ImGui::Separator();
        if (ImGui::BeginTable("PreparationCommands", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Key");
            ImGui::TableSetupColumn("Action");
            ImGui::TableHeadersRow();
            auto row = [](const char* key, const char* action) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(action);
            };
            row("P", "Add plane");
            row("B", "Add room planes");
            row("C", "Select next plane");
            row("N", "Start point-based plane draft");
            row("M", "Add draft point");
            row("F", "Finalize draft");
            row("V", "Edit selected plane as draft");
            row("X", "Cancel draft");
            row("I/K/J/L/U/O", "Move selected object or draft cursor");
            ImGui::EndTable();
        }

        ImGui::Spacing();
        if (ImGui::Button("Close")) {
            showCommandsModal_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (!modalOpen) {
        showCommandsModal_ = false;
    }
}

Vec3 App::simulationCameraPosition() const
{
    return camera_.position();
}

void App::syncSimulationCameraViewMode()
{
    const SimulationViewMode currentViewMode = simulationScreen_.viewMode();
    if (currentViewMode == lastSimulationViewMode_) {
        return;
    }

    lastSimulationViewMode_ = currentViewMode;
    if (currentViewMode != SimulationViewMode::Internal) {
        return;
    }

    Vec3 roomCenter{0.0f, 0.0f, 0.0f};
    int centerSampleCount = 0;
    for (const GuiPlane& plane : scenario_.planes) {
        roomCenter.x += plane.center.x;
        roomCenter.y += plane.center.y;
        roomCenter.z += plane.center.z;
        ++centerSampleCount;
    }
    if (centerSampleCount > 0) {
        roomCenter.x /= static_cast<float>(centerSampleCount);
        roomCenter.y /= static_cast<float>(centerSampleCount);
        roomCenter.z /= static_cast<float>(centerSampleCount);
    }

    Vec3 eye = roomCenter;
    if (!scenario_.receivers.empty()) {
        const Vec3 receiverPosition = scenario_.receivers.front().position;
        eye = {receiverPosition.x, receiverPosition.y + 0.25f, receiverPosition.z};
    } else if (!scenario_.sources.empty()) {
        const Vec3 sourcePosition = scenario_.sources.front().position;
        eye = {sourcePosition.x, sourcePosition.y + 0.25f, sourcePosition.z};
    }

    Vec3 target{roomCenter.x, roomCenter.y, roomCenter.z - 1.0f};
    if (!scenario_.sources.empty()) {
        target = scenario_.sources.front().position;
    } else if (!scenario_.planes.empty()) {
        target = roomCenter;
    }

    const float dx = target.x - eye.x;
    const float dy = target.y - eye.y;
    const float dz = target.z - eye.z;
    if ((dx * dx + dy * dy + dz * dz) <= 0.0001f) {
        target = {eye.x, eye.y, eye.z - 1.0f};
    }

    camera_.setPose(eye, target);
}

void App::updateWindowTitle()
{
    std::string title = "Acoustic Simulator - ";
    if (mode_ == AppMode::Preparation) {
        if (selection_.isGeneralSelected()) {
            title += "Preparation Mode [General] [Tab: Simulation]";
        } else if (selection_.isSourceSelected()) {
            title += "Preparation Mode [Source " + std::to_string(preparationScreen_.selectedSourceId()) + "] [Tab: Simulation]";
        } else if (selection_.isReceiverSelected()) {
            title += "Preparation Mode [Receiver " + std::to_string(preparationScreen_.selectedReceiverId()) + "] [Tab: Simulation]";
        } else {
            title += "Preparation Mode [Plane " + std::to_string(preparationScreen_.selectedPlaneId()) + "] [Tab: Simulation]";
        }
    } else if (!simulationScreen_.isPrecomputeReady()) {
        title += "Simulation Mode - Preparing [Tab: Preparation]";
    } else if (simulationScreen_.isFinished()) {
        title += "Simulation Mode - Finished [Enter: Restart] [Tab: Preparation]";
    } else if (simulationScreen_.isRunning()) {
        title += "Simulation Mode - Running [Tab: Preparation]";
    } else {
        title += "Simulation Mode - Ready [Enter: Start] [Tab: Preparation]";
    }

    glfwSetWindowTitle(window_, title.c_str());
}

}  // namespace gui
