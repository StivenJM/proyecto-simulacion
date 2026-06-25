cmake_minimum_required(VERSION 3.20)

project(AcousticSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(AcousticSimulator
    src/main.cpp
    src/gui/App.cpp
    src/gui/App.h
    src/gui/AppMode.h
    src/gui/composition/GuiComposition.cpp
    src/gui/composition/GuiComposition.h
    src/gui/composition/GuiServices.h
    src/gui/config/GuiConfig.h
    src/gui/config/GuiServiceProvider.h
    src/gui/entities/GuiPlane.h
    src/gui/entities/GuiPlaneDraft.h
    src/gui/entities/GuiReceiver.h
    src/gui/entities/GuiScenario.h
    src/gui/entities/GuiSelection.h
    src/gui/entities/GuiSource.h
    src/gui/entities/GuiTriangle.h
    src/gui/factories/PlaneServiceFactory.cpp
    src/gui/factories/PlaneServiceFactory.h
    src/gui/factories/ScenarioServiceFactory.cpp
    src/gui/factories/ScenarioServiceFactory.h
    src/gui/input/CameraController.cpp
    src/gui/input/CameraController.h
    src/gui/input/InputState.cpp
    src/gui/input/InputState.h
    src/gui/rendering/GuiScenarioRenderMapper.cpp
    src/gui/rendering/GuiScenarioRenderMapper.h
    src/gui/rendering/OpenGLRenderer.cpp
    src/gui/rendering/OpenGLRenderer.h
    src/gui/rendering/RenderTypes.h
    src/gui/ui/ImGuiLayer.cpp
    src/gui/ui/ImGuiLayer.h
    src/gui/screens/components/AxisGizmo.cpp
    src/gui/screens/components/AxisGizmo.h
    src/gui/screens/preparation/PreparationScreen.cpp
    src/gui/screens/preparation/PreparationScreen.h
    src/gui/screens/preparation/components/PlaneEditorControls.cpp
    src/gui/screens/preparation/components/PlaneEditorControls.h
    src/gui/screens/preparation/components/SceneHierarchyPanel.cpp
    src/gui/screens/preparation/components/SceneHierarchyPanel.h
    src/gui/screens/preparation/components/PlaneCreationPanel.cpp
    src/gui/screens/preparation/components/PlaneCreationPanel.h
    src/gui/screens/preparation/components/PlanePropertiesPanel.cpp
    src/gui/screens/preparation/components/PlanePropertiesPanel.h
    src/gui/screens/preparation/components/PlaneEditorPanel.cpp
    src/gui/screens/preparation/components/PlaneEditorPanel.h
    src/gui/screens/simulation/SimulationScreen.cpp
    src/gui/screens/simulation/SimulationScreen.h
    src/gui/services/IPlaneService.h
    src/gui/services/IScenarioService.h
    src/gui/services/ISimulationService.h
    src/gui/services/dtos/PlaneDtos.h
    src/gui/services/dtos/ScenarioDtos.h
    src/gui/services/dtos/SimulationDtos.h
    src/gui/services/implementations/core/CorePlaneService.cpp
    src/gui/services/implementations/core/CorePlaneService.h
    src/gui/services/implementations/core/CoreScenarioService.cpp
    src/gui/services/implementations/core/CoreScenarioService.h
    src/gui/services/implementations/mock/MockPlaneService.cpp
    src/gui/services/implementations/mock/MockPlaneService.h
    src/gui/services/implementations/mock/MockScenarioService.cpp
    src/gui/services/implementations/mock/MockScenarioService.h
)

target_include_directories(AcousticSimulator
    PRIVATE
    src
)

target_link_libraries(AcousticSimulator
    PRIVATE
    glfw
    glad::glad
    imgui::imgui
    OpenGL::GL
)
