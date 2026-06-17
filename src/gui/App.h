#pragma once

#include "AppMode.h"
#include "CameraController.h"
#include "OpenGLRenderer.h"
#include "SceneEditor.h"

struct GLFWwindow;

namespace gui {

class App {
public:
    App() = default;
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool initialize();
    void run();

private:
    static void onFramebufferResize(GLFWwindow* window, int width, int height);

    void resize(int width, int height);
    void processInput();
    void processPreparationInput(float deltaTime);
    void toggleMode();
    void startSimulation();
    void updateWindowTitle();

    bool isKeyPressedOnce(int key, bool& previousState) const;

    GLFWwindow* window_ = nullptr;
    CameraController camera_;
    SceneEditor sceneEditor_;
    OpenGLRenderer renderer_;
    AppMode mode_ = AppMode::Preparation;
    bool simulationStarted_ = false;
    bool tabWasPressed_ = false;
    bool enterWasPressed_ = false;
    bool pWasPressed_ = false;
    bool bWasPressed_ = false;
    bool cWasPressed_ = false;
};

}  // namespace gui
