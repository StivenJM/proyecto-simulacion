#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

namespace gui {

App::~App()
{
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
    return renderer_.initialize();
}

void App::run()
{
    float previousTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window_)) {
        const float currentTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        processInput();
        camera_.update(window_, deltaTime);
        renderer_.render(camera_.viewProjectionMatrix(), mode_, simulationStarted_);

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

void App::processInput()
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }

    if (isKeyPressedOnce(GLFW_KEY_TAB, tabWasPressed_)) {
        toggleMode();
    }

    if (isKeyPressedOnce(GLFW_KEY_ENTER, enterWasPressed_)) {
        startSimulation();
    }
}

void App::toggleMode()
{
    if (mode_ == AppMode::Preparation) {
        mode_ = AppMode::Simulation;
    } else {
        mode_ = AppMode::Preparation;
        simulationStarted_ = false;
    }

    updateWindowTitle();
}

void App::startSimulation()
{
    if (mode_ != AppMode::Simulation) {
        std::cout << "Switch to simulation mode before starting the simulation.\n";
        return;
    }

    if (!simulationStarted_) {
        simulationStarted_ = true;
        updateWindowTitle();
        std::cout << "Simulation started. Preparation editing is locked.\n";
    }
}

void App::updateWindowTitle()
{
    std::string title = "Acoustic Simulator - ";
    if (mode_ == AppMode::Preparation) {
        title += "Preparation Mode [Tab: Simulation]";
    } else if (simulationStarted_) {
        title += "Simulation Mode - Running [Tab: Preparation]";
    } else {
        title += "Simulation Mode - Ready [Enter: Start] [Tab: Preparation]";
    }

    glfwSetWindowTitle(window_, title.c_str());
}

bool App::isKeyPressedOnce(int key, bool& previousState) const
{
    const bool isPressed = glfwGetKey(window_, key) == GLFW_PRESS;
    const bool wasPressedNow = isPressed && !previousState;
    previousState = isPressed;
    return wasPressedNow;
}

}  // namespace gui
