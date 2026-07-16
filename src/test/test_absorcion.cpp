#include <gtest/gtest.h>
#include "services/implementations/core/DiffuseEnergySolver.h"
#include "services/implementations/core/GeometryCalculator.h"
#include "test_utils.h"

using namespace services;
using namespace services::core;
using namespace test_utils;

namespace {
std::vector<SurfaceData> makeSurfaces(const std::vector<std::pair<TriangleData,double>>& items) {
    std::vector<SurfaceData> surfaces;
    int sid = 0;
    for (auto& [tri, absorption] : items) {
        SurfaceData s;
        s.id = sid++;
        s.absorption = absorption;
        s.triangles = {tri};
        surfaces.push_back(s);
    }
    return surfaces;
}
}

// --- Absorcion = 0: toda la energia se conserva (sin perdida) --------------
TEST(Absorcion, AbsorcionCero_SinPerdida) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeSurfaces({{a, 0.0}, {b, 0.0}});

    SimulationConfig cfg = buildConfigS1(20); // 2 pasos de 10 ms
    double initial = 1.0;
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, initial, cfg);

    // En t=0, cada triangulo arranca con initial/2 = 0.5 (ver bug documentado
    // en test_reflexionDifusa.cpp sobre el reparto inicial uniforme).
    bool encontroT0 = false;
    for (auto& s : samples) {
        if (s.triangleId == 0 && s.timeMs == 0) {
            EXPECT_NEAR(s.energy, 0.5, kEpsilon);
            encontroT0 = true;
        }
    }
    EXPECT_TRUE(encontroT0);
}

// --- Absorcion = 1: toda la energia se pierde en la primera transicion -----
TEST(Absorcion, AbsorcionTotal_PierdeToda) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    auto surfaces = makeSurfaces({{a, 1.0}, {b, 1.0}});

    SimulationConfig cfg = buildConfigS1(50);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    // Con absorcion=1.0, afterAbsorb=0 en el primer paso, por lo que no
    // deberia haber NINGUNA muestra registrada en t=10ms o posterior (el
    // umbral de corte es 1e-10 y la energia cae exactamente a 0).
    for (auto& s : samples) {
        EXPECT_LT(s.timeMs, 10)
            << "Con absorcion total, no deberia propagarse energia mas alla de t=0";
    }
}

// --- Formula multiplicativa exacta: E_after = E*(1-absorcion) --------------
TEST(Absorcion, FormulaMultiplicativaExacta) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    double absorcion = 0.3;
    auto surfaces = makeSurfaces({{a, absorcion}, {b, absorcion}});

    SimulationConfig cfg = buildConfigS1(10); // un solo paso adicional
    double initial = 1.0;
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, initial, cfg);

    double energiaT1_en_t10 = -1.0;
    for (auto& s : samples) {
        if (s.triangleId == 1 && s.timeMs == 10) energiaT1_en_t10 = s.energy;
    }
    ASSERT_GE(energiaT1_en_t10, 0.0) << "No se encontro muestra para T1 en t=10ms";
    EXPECT_NEAR(energiaT1_en_t10, 0.35, 1e-6);
}

// --- getAbsorption: triangulo desconocido usa el valor por defecto 0.1 -----
TEST(Absorcion, TrianguloDesconocidoUsaDefaultPuntoUno) {
    TriangleData a = T0_piso();
    TriangleData b = T1_oeste();
    std::vector<TriangleData> tris = {a, b};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);

    SurfaceData onlySurfaceForB;
    onlySurfaceForB.id = 0;
    onlySurfaceForB.absorption = 0.9; // no deberia usarse para T0
    onlySurfaceForB.triangles = {b};
    std::vector<SurfaceData> surfaces = {onlySurfaceForB};

    SimulationConfig cfg = buildConfigS1(10);
    auto samples = DiffuseEnergySolver::solve(diff, tris, surfaces, 1.0, cfg);

    // energy[T0] en t=0 = 0.5 ; afterAbsorb(T0) usando default 0.1 = 0.45
    double energiaT1_en_t10 = -1.0;
    for (auto& s : samples) {
        if (s.triangleId == 1 && s.timeMs == 10) energiaT1_en_t10 = s.energy;
    }
    ASSERT_GE(energiaT1_en_t10, 0.0);
    EXPECT_NEAR(energiaT1_en_t10, 0.45, 1e-6)
        << "Se esperaba que T0 (ausente de la lista de superficies) usara "
           "la absorcion por defecto de 0.1, no la de la superficie de T1 (0.9)";
}
