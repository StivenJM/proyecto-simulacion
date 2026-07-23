#pragma once

#include "AppMode.h"
#include "composition/GuiServices.h"
#include "entities/GuiScenario.h"
#include "entities/GuiSelection.h"
#include "input/CameraController.h"
#include "input/InputState.h"
#include "rendering/GuiScenarioRenderMapper.h"
#include "rendering/OpenGLRenderer.h"
#include "screens/preparation/components/SceneHierarchyPanel.h"
#include "screens/preparation/components/PlaneEditorPanel.h"
#include "screens/preparation/PreparationScreen.h"
#include "screens/simulation/SimulationScreen.h"
#include "ui/ImGuiLayer.h"

struct GLFWwindow;

namespace gui {

enum class InteractionMode {
    Ui,
    Camera,
};

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
    bool processInput(bool imguiWantsKeyboard);
    void toggleMode();
    void toggleInteractionMode();
    void setInteractionMode(InteractionMode mode);
    void renderInteractionModeHud();
    void renderCommandsModal();
    bool startSimulation();
    void syncSimulationCameraViewMode();
    Vec3 simulationCameraPosition() const;
    Mat4 simulationViewProjection() const;
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
    ImGuiLayer imguiLayer_;
    SceneHierarchyPanel sceneHierarchyPanel_;
    PlaneEditorPanel planeEditorPanel_;
    AppMode mode_ = AppMode::Preparation;
    SimulationViewMode lastSimulationViewMode_ = SimulationViewMode::External;
    InteractionMode interactionMode_ = InteractionMode::Ui;
    bool showCommandsModal_ = false;
};

}  // namespace gui
