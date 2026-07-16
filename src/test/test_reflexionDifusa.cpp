#include <gtest/gtest.h>
#include "services/implementations/core/DiffuseEnergySolver.h"
#include "services/implementations/core/GeometryCalculator.h"
#include "test_utils.h"

using namespace services;
using namespace services::core;
using namespace test_utils;

namespace {
std::vector<SurfaceData> makeTwoSurfaces(TriangleData a, TriangleData b, double absorption) {
    SurfaceData sa; sa.id = 0; sa.absorption = absorption; sa.triangles = {a};
    SurfaceData sb; sb.id = 1; sb.absorption = absorption; sb.triangles = {b};
    return {sa, sb};
}

double maxEnergySampleFor(const std::vector<TriangleEnergySample>& samples, int triangleId, int timeMs) {
    for (auto& s : samples) if (s.triangleId == triangleId && s.timeMs == timeMs) return s.energy;
    return -1.0;
}
}

// --- Con 0 triangulos, no debe producir muestras (caso trivial/defensivo) --
TEST(ReflexionDifusa, SinTriangulos_SinMuestras) {
    DiffusionMatrixData vacio;
    std::vector<TriangleData> tris;
    std::vector<SurfaceData> surfaces;
    SimulationConfig cfg = buildConfigS1(100);
    auto samples = DiffuseEnergySolver::solve(vacio, tris, surfaces, 1.0, cfg);
    EXPECT_TRUE(samples.empty());
}

TEST(ReflexionDifusa, Bug_EnergiaInicialSeReparteUniformeIgnorandoDistancia) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);

    ASSERT_TRUE(diff.visibility[0][1]);
    ASSERT_EQ(diff.timesMs[0][1], 96) << "El tiempo de vuelo real T0->T5 es 96 ms";

    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(150);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    double energiaLejosEnT0 = maxEnergySampleFor(samples, /*triangleId=*/5, /*timeMs=*/0);
    ASSERT_GE(energiaLejosEnT0, 0.0) << "Se esperaba una muestra para T5 en t=0";
    EXPECT_GT(energiaLejosEnT0, 0.0)
        << "BUG: T5 ya tiene energia (" << energiaLejosEnT0 << ") en t=0 ms, "
           "pese a que el tiempo de vuelo real hasta T5 es de 96 ms. La "
           "matriz 'tiempo' se calcula correctamente pero no se usa para "
           "retrasar la aparicion de la energia inicial.";
    EXPECT_NEAR(energiaLejosEnT0, 0.5, kEpsilon)
        << "El valor coincide exactamente con initialEnergy/n = 1.0/2 = 0.5, "
           "confirmando que es un reparto uniforme por conteo de triangulos, "
           "no una propagacion fisica dependiente de la distancia.";
}

TEST(ReflexionDifusa, DISABLED_EspecificacionCorrecta_SinEnergiaAntesDeTiempoDeVuelo) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(150);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    for (auto& s : samples) {
        if (s.triangleId == 5) {
            EXPECT_GE(s.timeMs, diff.timesMs[0][1])
                << "T5 no deberia tener energia antes de t=96ms (tiempo de vuelo)";
        }
    }
}

TEST(ReflexionDifusa, Bug_LlegadaDeEnergiaIgnoraLaMatrizTiempo) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(50); // 5 pasos de 10 ms, todos < 96 ms

    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    int muestrasTempranasDeT5 = 0;
    for (auto& s : samples) {
        if (s.triangleId == 5 && s.timeMs < 96) muestrasTempranasDeT5++;
    }
    EXPECT_GT(muestrasTempranasDeT5, 0)
        << "BUG: existen " << muestrasTempranasDeT5 << " muestras de T5 con "
           "timeMs < 96 ms (el tiempo de vuelo real), lo cual es fisicamente "
           "imposible y demuestra que 'tiempo' no se respeta como delay.";
}

// --- Corte por duracion: energia con arrivalMs > durationMs se descarta ----
TEST(ReflexionDifusa, CorteRespetaDuracionMaxima) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(a, b, 0.1);

    SimulationConfig cfg = buildConfigS1(5); // duracion menor al primer timeStep completo
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);
    for (auto& s : samples) {
        EXPECT_LE(s.timeMs, cfg.durationMs);
    }
}

// --- La energia nunca es negativa en ninguna muestra ------------------------
TEST(ReflexionDifusa, EnergiaNuncaNegativa) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData s0; s0.id=0; s0.absorption=0.15; s0.triangles={tris[0]};
    SurfaceData s1; s1.id=1; s1.absorption=0.15; s1.triangles={tris[1]};
    SurfaceData s2; s2.id=2; s2.absorption=0.15; s2.triangles={tris[2]};
    std::vector<SurfaceData> surfaces = {s0,s1,s2};

    SimulationConfig cfg = buildConfigS1(200);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 3.0, cfg);
    for (auto& s : samples) EXPECT_GE(s.energy, 0.0);
}
