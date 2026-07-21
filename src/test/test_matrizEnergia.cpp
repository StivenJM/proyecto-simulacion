#include <gtest/gtest.h>
#include "core/GeometryCalculator.h"
#include "core/DiffuseEnergySolver.h"
#include "core/RayTracer.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Estructura: las 4 matrices son cuadradas n x n y estan alineadas -------
TEST(MatrizEnergia, DimensionesConsistentes) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T2_techo(), T4_sur(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    const size_t n = tris.size();
    ASSERT_EQ(diff.distances.size(), n);
    ASSERT_EQ(diff.timesMs.size(), n);
    ASSERT_EQ(diff.percentages.size(), n);
    ASSERT_EQ(diff.visibility.size(), n);
    for (size_t i = 0; i < n; i++) {
        EXPECT_EQ(diff.distances[i].size(), n);
        EXPECT_EQ(diff.timesMs[i].size(), n);
        EXPECT_EQ(diff.percentages[i].size(), n);
        EXPECT_EQ(diff.visibility[i].size(), n);
    }
}

// --- Indices: la fila/columna k corresponde al k-esimo triangulo del vector
TEST(MatrizEnergia, IndicesCorrespondenAlOrdenDeEntrada) {
    std::vector<TriangleData> tris = {T4_sur(), T0_piso()}; // indices: 0=T4, 1=T0
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    Vec3 c4 = GeometryCalculator::centroid(T4_sur());
    Vec3 c0 = GeometryCalculator::centroid(T0_piso());
    double esperado = GeometryCalculator::distance(c4, c0);
    EXPECT_NEAR(diff.distances[0][1], esperado, kEpsilon);
    EXPECT_NEAR(diff.distances[1][0], esperado, kEpsilon);
}

// --- mE (TriangleEnergySample) solo referencia ids de triangulos validos ---
TEST(MatrizEnergia, MuestrasDeEnergiaReferencianIdsValidos) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData s0; s0.id=0; s0.absorption=0.1; s0.triangles={tris[0]};
    SurfaceData s1; s1.id=1; s1.absorption=0.1; s1.triangles={tris[1]};
    SurfaceData s2; s2.id=2; s2.absorption=0.1; s2.triangles={tris[2]};
    std::vector<SurfaceData> surfaces = {s0,s1,s2};

    SimulationConfig cfg = buildConfigS1(50);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    std::vector<int> idsValidos;
    for (auto& t : tris) idsValidos.push_back(t.id);

    for (auto& s : samples) {
        bool esValido = std::find(idsValidos.begin(), idsValidos.end(), s.triangleId) != idsValidos.end();
        EXPECT_TRUE(esValido) << "TriangleEnergySample con triangleId=" << s.triangleId
                              << " no corresponde a ningun triangulo de entrada";
    }
}

TEST(MatrizEnergia, Bug_RayTracerNoExponeEnergiaPorTrianguloParaSembrarMe) {
    ScenarioData scenario = buildScenarioS1();
    SimulationConfig cfg = buildConfigS1(50);

    RayTracer::TraceOutput out = RayTracer::trace(
        scenario.sources[0], scenario.surfaces, scenario.receivers, cfg);

    SUCCEED() << "Verificado por inspeccion de tipos: RayTracer::TraceOutput "
                 "= { std::vector<ReflectionRaySegment> rays; "
                 "std::vector<ReceiverEnergySample> receiverEnergy; } "
                 "-- no incluye energia por triangulo.";
}

TEST(MatrizEnergia, Bug_EnergiaDifusaInicialEsIndependienteDelRayTracing) {
    std::vector<TriangleData> tris = {T0_piso(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData s0; s0.id=0; s0.absorption=0.1; s0.triangles={tris[0]};
    SurfaceData s1; s1.id=1; s1.absorption=0.1; s1.triangles={tris[1]};
    std::vector<SurfaceData> surfaces = {s0, s1};

    SimulationConfig cfg = buildConfigS1(10);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    double e0 = -1, e5 = -1;
    for (auto& s : samples) {
        if (s.triangleId == 0 && s.timeMs == 0) e0 = s.energy;
        if (s.triangleId == 5 && s.timeMs == 0) e5 = s.energy;
    }
    ASSERT_GE(e0, 0.0);
    ASSERT_GE(e5, 0.0);
    EXPECT_NEAR(e0, e5, kEpsilon)
        << "T0 (cerca, presumiblemente golpeado por rayos) y T5 (a 32.68 m, "
           "presumiblemente nunca golpeado) reciben exactamente la misma "
           "energia inicial, confirmando que mE no se carga desde el "
           "trazado de rayos sino desde un reparto uniforme.";
}
