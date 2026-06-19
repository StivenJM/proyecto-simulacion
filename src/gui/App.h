#pragma once

#include "AppMode.h"
#include "composition/GuiServices.h"
#include "entities/GuiScenario.h"
#include "entities/GuiSelection.h"
#include "input/CameraController.h"
#include "input/InputState.h"
#include "rendering/GuiScenarioRenderMapper.h"
#include "rendering/OpenGLRenderer.h"
#include "screens/preparation/PreparationScreen.h"
#include "screens/simulation/SimulationScreen.h"

struct GLFWwindow;

namespace gui {

class App {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool initialize();
    void run();

private:
    static void onFramebufferResize(GLFWwindow* window, int width, int height);

    void resize(int width, int height);
    void processInput();
    void toggleMode();
    void startSimulation();
    void updateWindowTitle();

    GLFWwindow* window_ = nullptr;
    GuiServices services_;
    GuiSelection selection_;
    GuiScenario scenario_;
    PreparationScreen preparationScreen_;
    SimulationScreen simulationScreen_;
    GuiScenarioRenderMapper renderMapper_;
    InputState input_;
    CameraController camera_;
    OpenGLRenderer renderer_;
    AppMode mode_ = AppMode::Preparation;
};

}  // namespace gui
