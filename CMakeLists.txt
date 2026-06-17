cmake_minimum_required(VERSION 3.20)

project(AcousticSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(AcousticSimulator
    src/main.cpp
    src/gui/App.cpp
    src/gui/App.h
    src/gui/AppMode.h
    src/gui/CameraController.cpp
    src/gui/CameraController.h
    src/gui/OpenGLRenderer.cpp
    src/gui/OpenGLRenderer.h
    src/gui/RenderTypes.h
    src/gui/SceneEditor.cpp
    src/gui/SceneEditor.h
)

target_include_directories(AcousticSimulator
    PRIVATE
    src
)

target_link_libraries(AcousticSimulator
    PRIVATE
    glfw
    glad::glad
    OpenGL::GL
)
