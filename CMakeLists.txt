cmake_minimum_required(VERSION 3.20)

project(HelloWindow)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(HelloWindow
    src/main.cpp
)

target_link_libraries(HelloWindow
    PRIVATE
    glfw
    glad::glad
    OpenGL::GL
)