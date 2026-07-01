cmake_minimum_required(VERSION 3.20)

project(AcousticSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# ── Capa config ──────────────────────────────────────────────────────────────
add_library(app_config STATIC
    src/config/AppConfig.cpp
    src/config/AppConfig.h
    src/config/ServiceMode.h
)
target_include_directories(app_config PUBLIC src)

# ── Capa services (tipos, interfaz, factory) ─────────────────────────────────
add_library(simulation_services STATIC
    src/services/ServiceFactory.cpp
    src/services/ServiceFactory.h
    src/services/SimulationTypes.h
    src/services/ISimulationService.h
)
target_include_directories(simulation_services PUBLIC src)
target_link_libraries(simulation_services PUBLIC app_config)

# ── Mock ─────────────────────────────────────────────────────────────────────
add_library(simulation_service_mock STATIC
    src/services/implementations/mock/MockSimulationService.cpp
    src/services/implementations/mock/MockSimulationService.h
)
target_include_directories(simulation_service_mock PUBLIC src)
target_link_libraries(simulation_service_mock PUBLIC simulation_services)

# ── Core (sin OpenGL) ─────────────────────────────────────────────────────────
add_library(simulation_service_core STATIC
    src/services/implementations/core/GeometryCalculator.cpp
    src/services/implementations/core/GeometryCalculator.h
    src/services/implementations/core/RayTracer.cpp
    src/services/implementations/core/RayTracer.h
    src/services/implementations/core/DiffuseEnergySolver.cpp
    src/services/implementations/core/DiffuseEnergySolver.h
    src/services/implementations/core/CoreSimulationService.cpp
    src/services/implementations/core/CoreSimulationService.h
)
target_include_directories(simulation_service_core PUBLIC src)
target_link_libraries(simulation_service_core PUBLIC simulation_services)

# ── Ejecutable principal ──────────────────────────────────────────────────────
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
    src/gui/factories/ReceiverServiceFactory.cpp
    src/gui/factories/ReceiverServiceFactory.h
    src/gui/factories/ScenarioServiceFactory.cpp
    src/gui/factories/ScenarioServiceFactory.h
    src/gui/factories/SourceServiceFactory.cpp
    src/gui/factories/SourceServiceFactory.h
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
    src/gui/screens/preparation/components/ReceiverPropertiesPanel.cpp
    src/gui/screens/preparation/components/ReceiverPropertiesPanel.h
    src/gui/screens/preparation/components/PlaneEditorPanel.cpp
    src/gui/screens/preparation/components/PlaneEditorPanel.h
    src/gui/screens/preparation/components/SourcePropertiesPanel.cpp
    src/gui/screens/preparation/components/SourcePropertiesPanel.h
    src/gui/screens/simulation/SimulationScreen.cpp
    src/gui/screens/simulation/SimulationScreen.h
    src/gui/services/IPlaneService.h
    src/gui/services/IReceiverService.h
    src/gui/services/IScenarioService.h
    src/gui/services/ISimulationService.h
    src/gui/services/ISourceService.h
    src/gui/services/dtos/PlaneDtos.h
    src/gui/services/dtos/ScenarioDtos.h
    src/gui/services/dtos/SimulationDtos.h
    src/gui/services/implementations/core/CorePlaneService.cpp
    src/gui/services/implementations/core/CorePlaneService.h
    src/gui/services/implementations/core/CoreReceiverService.cpp
    src/gui/services/implementations/core/CoreReceiverService.h
    src/gui/services/implementations/core/CoreScenarioService.cpp
    src/gui/services/implementations/core/CoreScenarioService.h
    src/gui/services/implementations/core/CoreSourceService.cpp
    src/gui/services/implementations/core/CoreSourceService.h
    src/gui/services/implementations/mock/MockPlaneService.cpp
    src/gui/services/implementations/mock/MockPlaneService.h
    src/gui/services/implementations/mock/MockReceiverService.cpp
    src/gui/services/implementations/mock/MockReceiverService.h
    src/gui/services/implementations/mock/MockScenarioService.cpp
    src/gui/services/implementations/mock/MockScenarioService.h
    src/gui/services/implementations/mock/MockSourceService.cpp
    src/gui/services/implementations/mock/MockSourceService.h
)

target_include_directories(AcousticSimulator PRIVATE src)

target_link_libraries(AcousticSimulator
    PRIVATE
    simulation_service_mock
    simulation_service_core
    simulation_services
    app_config
    glfw
    glad::glad
    imgui::imgui
    OpenGL::GL
)
