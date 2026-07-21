#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include "core/CoreSimulationService.h"
#include "core/GeometryCalculator.h"
#include "core/DiffuseEnergySolver.h"
#include "core/RayTracer.h"
#include "core/SurfaceTriangulator.h"
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
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(tris[0], 1.0), cfg);

    std::vector<int> idsValidos;
    for (auto& t : tris) idsValidos.push_back(t.id);

    for (auto& s : samples) {
        bool esValido = std::find(idsValidos.begin(), idsValidos.end(), s.triangleId) != idsValidos.end();
        EXPECT_TRUE(esValido) << "TriangleEnergySample con triangleId=" << s.triangleId
                              << " no corresponde a ningun triangulo de entrada";
    }
}

TEST(MatrizEnergia, RayTracerExponeSemillasDifusasPorImpacto) {
    ScenarioData scenario = buildScenarioS1();
    SimulationConfig cfg = buildConfigS1(50);
    std::vector<SurfaceData> coreSurfaces = SurfaceTriangulator::triangulateSurfaces(
        scenario.surfaces,
        cfg.meshSubdivisions
    );

    RayTracer::TraceOutput out = RayTracer::trace(
        scenario.sources[0], coreSurfaces, scenario.receivers, cfg);

    ASSERT_FALSE(out.diffuseSeeds.empty());
    for (const auto& seed : out.diffuseSeeds) {
        EXPECT_GT(seed.energy, 0.0);
        EXPECT_LE(seed.timeMs, cfg.durationMs);
        bool foundCoreTriangle = false;
        for (const auto& surface : coreSurfaces) {
            for (const auto& triangle : surface.triangles) {
                if (triangle.id == seed.triangleId && triangle.surfaceId == seed.surfaceId) {
                    foundCoreTriangle = true;
                }
            }
        }
        EXPECT_TRUE(foundCoreTriangle) << "La semilla debe referenciar triangulos generados por core.";
    }
}

TEST(MatrizEnergia, EnergiaDifusaInicialUsaSemillasExplicitas) {
    std::vector<TriangleData> tris = {T0_piso(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    SurfaceData s0; s0.id=0; s0.absorption=0.1; s0.triangles={tris[0]};
    SurfaceData s1; s1.id=1; s1.absorption=0.1; s1.triangles={tris[1]};
    std::vector<SurfaceData> surfaces = {s0, s1};

    SimulationConfig cfg = buildConfigS1(10);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, seedEnergy(tris[1], 1.0), cfg);

    double e0 = -1, e5 = -1;
    for (auto& s : samples) {
        if (s.triangleId == 0 && s.timeMs == 0) e0 = s.energy;
        if (s.triangleId == 5 && s.timeMs == 0) e5 = s.energy;
    }
    EXPECT_LT(e0, 0.0)
        << "T0 no debe recibir energia en t=0 si no hay una semilla explicita alli.";
    ASSERT_GE(e5, 0.0);
}

TEST(MatrizEnergia, CoreSimulationServiceUsaTriangulosGeneradosPorCoreParaSemillas) {
    ScenarioData scenario;
    SurfaceData surface = makeSurfaceFromTriangle(7, T0_piso(), 0.1);
    surface.triangles = {makeTriangle(999, 7, {100,100,100}, {101,100,100}, {100,101,100})};
    scenario.surfaces = {surface};

    SourceData source;
    source.id = 0;
    source.position = {1.0, 1.0, 1.0};
    source.energy = 1.0;
    scenario.sources = {source};

    SimulationConfig cfg = buildConfigS1(100);
    cfg.rayCount = 162;

    CoreSimulationService service;
    SimulationResult result = service.runSimulation(scenario, cfg);

    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.triangleEnergy.empty());
    for (const auto& sample : result.triangleEnergy) {
        EXPECT_NE(sample.triangleId, 999)
            << "La difusion no debe usar triangulos preexistentes del escenario/GUI.";
    }
}
