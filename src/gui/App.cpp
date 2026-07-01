#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "composition/GuiComposition.h"

#include <iostream>
#include <string>

namespace gui {

App::App()
    : services_(GuiComposition::createServices({})),
      scenario_(services_.scenarioService->createInitialScenario(selection_)),
      preparationScreen_(scenario_, selection_, *services_.planeService, *services_.sourceService, *services_.receiverService)
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(1280, 720, "Acoustic Simulator", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Failed to create the GLFW window.\n";
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, &App::onFramebufferResize);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD.\n";
        return false;
    }

    resize(1280, 720);
    updateWindowTitle();

    std::cout << "Controls: Tab switches Preparation/Simulation mode, Enter starts simulation mode, Esc closes.\n";
    std::cout << "Preparation: P adds a rectangular plane, B adds a room, C selects next plane, I/K/J/L/U/O moves selected item.\n";
    std::cout << "Point planes: N starts draft, M adds cursor point, I/K/J/L/U/O moves draft cursor, F finalizes, V edits selected plane, X cancels draft.\n";
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
        processInput(imguiWantsKeyboard);

        imguiLayer_.beginFrame();
        if (mode_ == AppMode::Preparation) {
            sceneHierarchyPanel_.render(preparationScreen_);
            planeEditorPanel_.render(preparationScreen_);
            updateWindowTitle();
        } else {
            simulationScreen_.update(deltaTime, scenario_);
            simulationScreen_.renderPanel(scenario_);
            updateWindowTitle();
        }

        if (!imguiWantsKeyboard && mode_ == AppMode::Preparation && preparationScreen_.handleInput(input_, deltaTime)) {
            updateWindowTitle();
        }

        if (!imguiWantsKeyboard) {
            camera_.update(window_, deltaTime);
        }

        renderer_.render(
            simulationViewProjection(),
            renderMapper_.buildRenderScene(scenario_, selection_, preparationScreen_.draft(), simulationScreen_.renderOverlay()),
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

void App::processInput(bool imguiWantsKeyboard)
{
    if (input_.isDown(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window_, true);
    }

    if (imguiWantsKeyboard) {
        return;
    }

    if (input_.wasPressed(GLFW_KEY_TAB)) {
        toggleMode();
    }

    if (input_.wasPressed(GLFW_KEY_ENTER)) {
        startSimulation();
    }
}

void App::toggleMode()
{
    if (mode_ == AppMode::Preparation) {
        mode_ = AppMode::Simulation;
    } else {
        mode_ = AppMode::Preparation;
        simulationScreen_.reset();
    }

    updateWindowTitle();
}

void App::startSimulation()
{
    if (mode_ != AppMode::Simulation) {
        std::cout << "Switch to simulation mode before starting the simulation.\n";
        return;
    }

    if (simulationScreen_.start(scenario_)) {
        updateWindowTitle();
        std::cout << "Simulation started. Preparation editing is locked.\n";
    }
}

Mat4 App::simulationViewProjection() const
{
    if (mode_ != AppMode::Simulation || simulationScreen_.viewMode() == SimulationViewMode::External || scenario_.receivers.empty()) {
        return camera_.viewProjectionMatrix();
    }

    const Vec3 eye = scenario_.receivers.front().position;
    Vec3 target{eye.x, eye.y, eye.z - 1.0f};
    if (!scenario_.sources.empty()) {
        target = scenario_.sources.front().position;
    } else if (!scenario_.planes.empty()) {
        target = scenario_.planes.front().center;
    }

    return camera_.viewProjectionFrom({eye.x, eye.y + 0.25f, eye.z}, target, 68.0f);
}

void App::updateWindowTitle()
{
    std::string title = "Acoustic Simulator - ";
    if (mode_ == AppMode::Preparation) {
        if (selection_.isSourceSelected()) {
            title += "Preparation Mode [Source " + std::to_string(preparationScreen_.selectedSourceId()) + "] [Tab: Simulation]";
        } else if (selection_.isReceiverSelected()) {
            title += "Preparation Mode [Receiver " + std::to_string(preparationScreen_.selectedReceiverId()) + "] [Tab: Simulation]";
        } else {
            title += "Preparation Mode [Plane " + std::to_string(preparationScreen_.selectedPlaneId()) + "] [Tab: Simulation]";
        }
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
