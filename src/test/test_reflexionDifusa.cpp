#include <gtest/gtest.h>
#include "core/DiffuseEnergySolver.h"
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
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
    auto samples = DiffuseEnergySolver::solve(vacio, tris, surfaces, {}, cfg);
    EXPECT_TRUE(samples.empty());
}

TEST(ReflexionDifusa, SemillaExplicitaRespetaTiempoDeVuelo) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);

    ASSERT_TRUE(diff.visibility[0][1]);
    ASSERT_EQ(diff.timesMs[0][1], 96) << "El tiempo de vuelo real T0->T5 es 96 ms";

    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(150);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(cerca, 1.0), cfg);

    EXPECT_LT(maxEnergySampleFor(samples, /*triangleId=*/5, /*timeMs=*/0), 0.0)
        << "T5 no debe tener energia antes del tiempo de vuelo de la semilla.";
    EXPECT_GT(maxEnergySampleFor(samples, /*triangleId=*/5, /*timeMs=*/96), 0.0)
        << "T5 debe recibir energia recien al cumplirse el tiempo de vuelo.";
}

TEST(ReflexionDifusa, EspecificacionCorrecta_SinEnergiaAntesDeTiempoDeVuelo) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(150);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(cerca, 1.0), cfg);

    for (auto& s : samples) {
        if (s.triangleId == 5) {
            EXPECT_GE(s.timeMs, diff.timesMs[0][1])
                << "T5 no deberia tener energia antes del tiempo de vuelo de la semilla";
        }
    }
}

TEST(ReflexionDifusa, LlegadaDeEnergiaRespetaLaMatrizTiempo) {
    TriangleData cerca = T0_piso();
    TriangleData lejos = T5_lejano();
    std::vector<TriangleData> tris = {cerca, lejos};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(cerca, lejos, 0.2);
    SimulationConfig cfg = buildConfigS1(50); // 5 pasos de 10 ms, todos < 96 ms

    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(cerca, 1.0), cfg);

    int muestrasTempranasDeT5 = 0;
    for (auto& s : samples) {
        if (s.triangleId == 5 && s.timeMs < 96) muestrasTempranasDeT5++;
    }
    EXPECT_EQ(muestrasTempranasDeT5, 0)
        << "No deben existir muestras de T5 antes del tiempo de vuelo real.";
}

// --- Corte por duracion: energia con arrivalMs > durationMs se descarta ----
TEST(ReflexionDifusa, CorteRespetaDuracionMaxima) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeTwoSurfaces(a, b, 0.1);

    SimulationConfig cfg = buildConfigS1(5); // duracion menor al primer timeStep completo
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(a, 1.0), cfg);
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
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(tris[0], 3.0), cfg);
    for (auto& s : samples) EXPECT_GE(s.energy, 0.0);
}
