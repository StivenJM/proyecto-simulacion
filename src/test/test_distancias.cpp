#include <gtest/gtest.h>
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Misma posicion: distancia debe ser 0 -----------------------------------
TEST(Distancias, MismaPosicion) {
    Vec3 a{5.0, -2.0, 7.5};
    EXPECT_NEAR(GeometryCalculator::distance(a, a), 0.0, kEpsilon);
}

// --- Distancia conocida (terna pitagorica 3-4-5) ----------------------------
TEST(Distancias, DistanciaConocida345) {
    Vec3 a{0,0,0};
    Vec3 b{3,4,0};
    // d = sqrt(3^2+4^2) = sqrt(25) = 5
    EXPECT_NEAR(GeometryCalculator::distance(a, b), 5.0, kEpsilon);
}

TEST(Distancias, DistanciaConocida3D) {
    Vec3 a{1,1,1};
    Vec3 b{4,5,13};
    // d = sqrt((4-1)^2+(5-1)^2+(13-1)^2) = sqrt(9+16+144) = sqrt(169) = 13
    EXPECT_NEAR(GeometryCalculator::distance(a, b), 13.0, kEpsilon);
}

// --- Simetria: dist(A,B) == dist(B,A) ---------------------------------------
TEST(Distancias, Simetria) {
    Vec3 a{2.5, -3.1, 8.0};
    Vec3 b{-7.2, 4.4, -1.9};
    double dAB = GeometryCalculator::distance(a, b);
    double dBA = GeometryCalculator::distance(b, a);
    EXPECT_NEAR(dAB, dBA, kEpsilon);
}

// --- Escenario S1: distancias exactas calculadas a mano ---------------------
TEST(Distancias, EscenarioS1_PisoOeste) {
    Vec3 c0 = GeometryCalculator::centroid(T0_piso());
    Vec3 c1 = GeometryCalculator::centroid(T1_oeste());
    // dist = sqrt((4/3)^2 + (2/3)^2 + (2/3)^2) = sqrt(24/9) = sqrt(8/3)
    double esperado = std::sqrt(8.0 / 3.0);
    EXPECT_NEAR(GeometryCalculator::distance(c0, c1), esperado, kEpsilon);
    EXPECT_NEAR(esperado, 1.632993, 1e-6);
}

TEST(Distancias, EscenarioS1_PisoSur) {
    Vec3 c0 = GeometryCalculator::centroid(T0_piso());
    Vec3 c4 = GeometryCalculator::centroid(T4_sur());
    // dist = sqrt((2/3)^2 + (4/3)^2 + (4/3)^2) = sqrt(36/9) = sqrt(4) = 2.0 (exacto)
    EXPECT_NEAR(GeometryCalculator::distance(c0, c4), 2.0, kEpsilon);
}

// --- La matriz de distancias generada por buildDiffusionMatrix -------------
TEST(Distancias, MatrizDiagonalCero) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    ASSERT_EQ(diff.distances.size(), tris.size());
    for (size_t i = 0; i < tris.size(); i++) {
        ASSERT_EQ(diff.distances[i].size(), tris.size());
        EXPECT_NEAR(diff.distances[i][i], 0.0, kEpsilon)
            << "La diagonal distances[" << i << "][" << i << "] deberia ser 0";
    }
}

TEST(Distancias, MatrizEsSimetrica) {
    // Fisicamente dist(i,j) debe ser igual a dist(j,i); se verifica que la
    // matriz completa (no solo el calculo puntual) preserva la simetria.
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T2_techo(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    for (size_t i = 0; i < tris.size(); i++) {
        for (size_t j = 0; j < tris.size(); j++) {
            EXPECT_NEAR(diff.distances[i][j], diff.distances[j][i], kEpsilon)
                << "Asimetria en distances[" << i << "][" << j << "]";
        }
    }
}

TEST(Distancias, MatrizDimensionNxN) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T2_techo(), T3_oeste_rev(), T4_sur(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    EXPECT_EQ(diff.distances.size(), tris.size());
    EXPECT_EQ(diff.timesMs.size(), tris.size());
    EXPECT_EQ(diff.percentages.size(), tris.size());
    EXPECT_EQ(diff.visibility.size(), tris.size());
    for (size_t i = 0; i < tris.size(); i++) {
        EXPECT_EQ(diff.distances[i].size(), tris.size());
        EXPECT_EQ(diff.timesMs[i].size(), tris.size());
        EXPECT_EQ(diff.percentages[i].size(), tris.size());
        EXPECT_EQ(diff.visibility[i].size(), tris.size());
    }
}
