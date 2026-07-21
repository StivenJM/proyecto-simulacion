#include <gtest/gtest.h>
#include "core/CoreSimulationService.h"
#include "core/GeometryCalculator.h"
#include "core/SurfaceTriangulator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Caso de error: escenario sin fuentes ----------------------------------
TEST(Integracion, EscenarioSinFuentes_Falla) {
    ScenarioData scenario; // sin fuentes ni geometria
    SimulationConfig cfg = buildConfigS1();
    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.message.empty());
}

// --- Caso de error: escenario sin geometria (solo fuente) ------------------
TEST(Integracion, EscenarioSinGeometria_Falla) {
    ScenarioData scenario;
    SourceData src; src.id = 0; src.position = {0,0,0}; src.energy = 1.0;
    scenario.sources = {src};
    SimulationConfig cfg = buildConfigS1();
    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);
    EXPECT_FALSE(result.success);
}

// --- Ejecucion exitosa del Escenario S1 completo ---------------------------
TEST(Integracion, EscenarioS1_EjecutaConExito) {
    ScenarioData scenario = buildScenarioS1(/*absorption=*/0.1);
    SimulationConfig cfg = buildConfigS1(/*durationMs=*/100);
    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);

    ASSERT_TRUE(result.success) << result.message;
    // La matriz de difusion debe tener dimension 4x4: el core triangula las
    // cuatro superficies outlinePoints del escenario S1.
    EXPECT_EQ(result.diffusion.distances.size(), 4u);
}

// --- La matriz de difusion generada durante la integracion coincide con la
TEST(Integracion, MatrizDeDifusionCoincideConCalculoAislado) {
    ScenarioData scenario = buildScenarioS1(0.1);
    SimulationConfig cfg = buildConfigS1(100);
    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);
    ASSERT_TRUE(result.success);

    // Recalculamos la matriz de forma independiente usando los triangulos que
    // genera el core desde outlinePoints + meshSubdivisions.
    std::vector<TriangleData> allTris;
    const std::vector<SurfaceData> triangulated = SurfaceTriangulator::triangulateSurfaces(
        scenario.surfaces,
        cfg.meshSubdivisions
    );
    for (auto& surf : triangulated)
        for (auto& tri : surf.triangles) allTris.push_back(tri);
    auto diffEsperado = GeometryCalculator::buildDiffusionMatrix(allTris, cfg.soundSpeed);

    ASSERT_EQ(result.diffusion.distances.size(), diffEsperado.distances.size());
    for (size_t i = 0; i < diffEsperado.distances.size(); i++) {
        for (size_t j = 0; j < diffEsperado.distances.size(); j++) {
            EXPECT_NEAR(result.diffusion.distances[i][j], diffEsperado.distances[i][j], kEpsilon);
            EXPECT_EQ(result.diffusion.timesMs[i][j], diffEsperado.timesMs[i][j]);
            EXPECT_EQ(result.diffusion.visibility[i][j], diffEsperado.visibility[i][j]);
            EXPECT_NEAR(result.diffusion.percentages[i][j], diffEsperado.percentages[i][j], kEpsilon);
        }
    }
}

// --- Conservacion de energia: energia recibida + energia perdida <= energia
// inicial de las fuentes (no puede haber creacion de energia de la nada) ---
TEST(Integracion, ConservacionDeEnergia_NoSeCreaEnergia) {
    ScenarioData scenario = buildScenarioS1(0.2);
    SimulationConfig cfg = buildConfigS1(100);
    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);
    ASSERT_TRUE(result.success);

    double totalInicial = 0.0;
    for (auto& s : scenario.sources) totalInicial += s.energy;
    EXPECT_NEAR(result.totalReceiverEnergy + result.lostEnergy, totalInicial, 1e-9);
    EXPECT_GE(result.lostEnergy, 0.0);
    EXPECT_GE(result.totalReceiverEnergy, 0.0);
}

// --- Determinismo: misma entrada produce la misma salida 
TEST(Integracion, Determinismo_MismaEntradaMismaSalida) {
    ScenarioData scenario = buildScenarioS1(0.15);
    SimulationConfig cfg = buildConfigS1(80);
    CoreSimulationService service;

    SimulationResult r1 = service.runSimulation(scenario, cfg);
    SimulationResult r2 = service.runSimulation(scenario, cfg);

    ASSERT_TRUE(r1.success);
    ASSERT_TRUE(r2.success);
    EXPECT_NEAR(r1.totalReceiverEnergy, r2.totalReceiverEnergy, 1e-12);
    EXPECT_NEAR(r1.lostEnergy, r2.lostEnergy, 1e-12);
    ASSERT_EQ(r1.reflectionRays.size(), r2.reflectionRays.size());
    ASSERT_EQ(r1.triangleEnergy.size(), r2.triangleEnergy.size());
}

TEST(Integracion, LinealidadAproximadaConLaEnergiaDeFuente) {
    ScenarioData escenarioBase = buildScenarioS1(0.1);
    ScenarioData escenarioDoble = buildScenarioS1(0.1);
    escenarioDoble.sources[0].energy = escenarioBase.sources[0].energy * 2.0;

    SimulationConfig cfg = buildConfigS1(60);
    CoreSimulationService service;

    SimulationResult rBase  = service.runSimulation(escenarioBase, cfg);
    SimulationResult rDoble = service.runSimulation(escenarioDoble, cfg);

    ASSERT_TRUE(rBase.success);
    ASSERT_TRUE(rDoble.success);

    if (rBase.totalReceiverEnergy > 1e-9) {
        double razon = rDoble.totalReceiverEnergy / rBase.totalReceiverEnergy;
        EXPECT_NEAR(razon, 2.0, 0.05)
            << "Se esperaba aproximadamente el doble de energia recibida al "
               "duplicar la energia de la fuente (sistema lineal)";
    } else {
        GTEST_SKIP() << "El escenario base no genero energia medible en el "
                        "receptor dentro de la duracion configurada; no se "
                        "puede evaluar la razon de linealidad.";
    }
}
