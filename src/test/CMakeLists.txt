cmake_minimum_required(VERSION 3.14)
project(Ambiente3D_Tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------
# GoogleTest (framework de pruebas). El proyecto original no traia ningun
# framework de testing propio, por lo que -segun lo solicitado- se usa
# GoogleTest, descargado automaticamente via FetchContent.
# ------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/v1.14.0
)
# Evita que GoogleTest sobreescriba las flags de compilador/enlazador del
# proyecto padre en Windows (no afecta Linux, se deja por buena practica).
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ------------------------------------------------------------------------
# Raiz del codigo fuente del proyecto auditado (Ambiente 3D v6).
# Se asume que esta carpeta tests/ vive dentro de la raiz del proyecto,
# junto a config/ y services/, tal como los #include originales lo exigen
# (ej. "config/AppConfig.h", "services/ISimulationService.h").
# ------------------------------------------------------------------------
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(ambiente3d_tests
  test_centroides.cpp
  test_distancias.cpp
  test_tiempo.cpp
  test_visibilidad.cpp
  test_porcentaje.cpp
  test_absorcion.cpp
  test_reflexionDifusa.cpp
  test_matrizEnergia.cpp
  test_integracion.cpp
  test_utils.cpp

  ${PROJECT_ROOT}/src/core/GeometryCalculator.cpp
  ${PROJECT_ROOT}/src/core/RayTracer.cpp
  ${PROJECT_ROOT}/src/core/DiffuseEnergySolver.cpp
  ${PROJECT_ROOT}/src/core/CoreSimulationService.cpp
)

target_include_directories(ambiente3d_tests PRIVATE
    ${PROJECT_ROOT}
    ${PROJECT_ROOT}/src
    ${PROJECT_ROOT}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ambiente3d_tests PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(ambiente3d_tests)
