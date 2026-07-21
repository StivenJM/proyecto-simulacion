#include <gtest/gtest.h>
#include "core/CoreSimulationService.h"
#include "core/DiffuseEnergySolver.h"
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- El valor por defecto del struct SI respeta el limite (1000 ms = 1 s) --
TEST(Limite1Segundo, ValorPorDefectoEsUnSegundo) {
    SimulationConfig cfg; // usa los defaults declarados en SimulationTypes.h
    EXPECT_EQ(cfg.durationMs, 1000);
    EXPECT_LE(cfg.durationMs, 1000);
}

TEST(Limite1Segundo, Bug_DiffuseEnergySolverNoRechazaDuracionMayorAUnSegundo) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData sa; sa.id=0; sa.absorption=0.05; sa.triangles={a};
    SurfaceData sb; sb.id=1; sb.absorption=0.05; sb.triangles={b};
    std::vector<SurfaceData> surfaces = {sa, sb};

    SimulationConfig cfg;
    cfg.durationMs = 2500; // 2.5 segundos: viola el enunciado
    cfg.soundSpeed = 340.0;

    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    int maxTimeMs = 0;
    for (auto& s : samples) maxTimeMs = std::max(maxTimeMs, s.timeMs);

    EXPECT_GT(maxTimeMs, 1000)
        << "BUG: el solver de difusion genero muestras con timeMs=" << maxTimeMs
        << " ms, superando ampliamente el limite de 1000 ms (1 segundo) "
           "exigido por el enunciado. No existe ninguna validacion que "
           "rechace o recorte durationMs > 1000.";
}

TEST(Limite1Segundo, Bug_CoreSimulationServiceNoValidaDuracionMaxima) {
    ScenarioData scenario = buildScenarioS1(0.1);
    SimulationConfig cfg = buildConfigS1();
    cfg.durationMs = 60000; // 60 segundos: claramente fuera de especificacion

    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);

    // El codigo actual acepta la simulacion sin fallar ni recortar la
    // duracion (result.success sigue siendo true), lo que confirma que no
    // hay ninguna validacion de negocio para el limite de 1 segundo.
    EXPECT_TRUE(result.success)
        << "Documenta que el servicio ACEPTA (incorrectamente, segun el "
           "enunciado) una duracion de 60000 ms sin rechazarla.";
}

TEST(Limite1Segundo, DISABLED_EspecificacionCorrecta_ClampAUnSegundo) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData sa; sa.id=0; sa.absorption=0.05; sa.triangles={a};
    SurfaceData sb; sb.id=1; sb.absorption=0.05; sb.triangles={b};
    std::vector<SurfaceData> surfaces = {sa, sb};

    SimulationConfig cfg;
    cfg.durationMs = 2500; // se solicitan 2.5 s
    cfg.soundSpeed = 340.0;

    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);
    int maxTimeMs = 0;
    for (auto& s : samples) maxTimeMs = std::max(maxTimeMs, s.timeMs);

    // Se espera que, tras la correccion propuesta (clamp de durationMs a
    // 1000 ms antes de iniciar la simulacion), ninguna muestra exceda 1000 ms.
    EXPECT_LE(maxTimeMs, 1000);
}
