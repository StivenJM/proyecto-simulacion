cmake_minimum_required(VERSION 3.20)

project(AcousticSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3  CONFIG REQUIRED)
find_package(glad   CONFIG REQUIRED)
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
    src/gui/CameraController.cpp
    src/gui/CameraController.h
    src/gui/OpenGLRenderer.cpp
    src/gui/OpenGLRenderer.h
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
    OpenGL::GL
)
