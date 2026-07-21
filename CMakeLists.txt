cmake_minimum_required(VERSION 3.20)

project(AcousticSimulator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ACOUSTIC_BUILD_TESTS "Build development-only tests" OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# ── Core (sin OpenGL) ─────────────────────────────────────────────────────────
add_library(simulation_core STATIC
    src/core/AppConfig.cpp
    src/core/ServiceFactory.cpp
    src/core/GeometryCalculator.cpp
    src/core/RayTracer.cpp
    src/core/DiffuseEnergySolver.cpp
    src/core/CoreSimulationService.cpp
    src/core/MockSimulationService.cpp
)
target_include_directories(simulation_core PUBLIC src/core src)

# ── Ejecutable principal ──────────────────────────────────────────────────────
add_executable(AcousticSimulator WIN32
    src/main.cpp
    src/app.rc
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
    src/gui/entities/GuiSimulationConfig.h
    src/gui/entities/GuiSource.h
    src/gui/entities/GuiTriangle.h
    src/gui/factories/PlaneServiceFactory.cpp
    src/gui/factories/PlaneServiceFactory.h
    src/gui/factories/ReceiverServiceFactory.cpp
    src/gui/factories/ReceiverServiceFactory.h
    src/gui/factories/ScenarioServiceFactory.cpp
    src/gui/factories/ScenarioServiceFactory.h
    src/gui/factories/SimulationServiceFactory.cpp
    src/gui/factories/SimulationServiceFactory.h
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
    src/gui/screens/preparation/components/GeneralPropertiesPanel.cpp
    src/gui/screens/preparation/components/GeneralPropertiesPanel.h
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
    src/gui/services/implementations/core/CoreSimulationService.cpp
    src/gui/services/implementations/core/CoreSimulationService.h
    src/gui/services/implementations/core/CoreSourceService.cpp
    src/gui/services/implementations/core/CoreSourceService.h
    src/gui/services/implementations/core/mappers/ResultMapper.cpp
    src/gui/services/implementations/core/mappers/ResultMapper.h
    src/gui/services/implementations/core/mappers/ScenarioMapper.cpp
    src/gui/services/implementations/core/mappers/ScenarioMapper.h
    src/gui/services/implementations/core/mappers/VectorMapper.cpp
    src/gui/services/implementations/core/mappers/VectorMapper.h
    src/gui/services/implementations/mock/MockPlaneService.cpp
    src/gui/services/implementations/mock/MockPlaneService.h
    src/gui/services/implementations/mock/MockReceiverService.cpp
    src/gui/services/implementations/mock/MockReceiverService.h
    src/gui/services/implementations/mock/MockScenarioService.cpp
    src/gui/services/implementations/mock/MockScenarioService.h
    src/gui/services/implementations/mock/MockSimulationService.cpp
    src/gui/services/implementations/mock/MockSimulationService.h
    src/gui/services/implementations/mock/MockSourceService.cpp
    src/gui/services/implementations/mock/MockSourceService.h
)

target_include_directories(AcousticSimulator PRIVATE src)

target_link_libraries(AcousticSimulator
    PRIVATE
    simulation_core
    glfw
    glad::glad
    imgui::imgui
    OpenGL::GL
)

# -- Tests de desarrollo ------------------------------------------------------
# Desactivados por defecto para no ensuciar el build/paquete original.
if(ACOUSTIC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/test)
endif()

# ── Instalador / paquete distribuible ───────────────────────────────────────
install(TARGETS AcousticSimulator RUNTIME DESTINATION bin)

if(WIN32)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    install(CODE "
        file(GLOB runtime_dlls \"${CMAKE_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/*.dll\")
        if(runtime_dlls)
            file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\" TYPE FILE FILES \${runtime_dlls})
        endif()
    ")
endif()

set(CPACK_PACKAGE_NAME "AcousticSimulator")
set(CPACK_PACKAGE_VENDOR "Proyecto Modelos")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Acoustic simulation desktop application")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AcousticSimulator")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win64")

if(WIN32)
    set(CPACK_GENERATOR "NSIS64;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Acoustic Simulator")
    set(CPACK_NSIS_PACKAGE_NAME "Acoustic Simulator")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
    set(CPACK_PACKAGE_EXECUTABLES "AcousticSimulator" "Acoustic Simulator")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "SetShellVarContext current\nCreateShortCut \\\"$DESKTOP\\\\Acoustic Simulator.lnk\\\" \\\"$INSTDIR\\\\bin\\\\AcousticSimulator.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\bin\\\\AcousticSimulator.exe\\\" 0"
    )
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "SetShellVarContext current\nDelete \\\"$DESKTOP\\\\Acoustic Simulator.lnk\\\""
    )
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
