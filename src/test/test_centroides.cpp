#include <gtest/gtest.h>
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Caso 1: triangulo "normal" (escaleno, valores enteros pequenos) -------
TEST(Centroide, TrianguloNormal) {
    // A=(0,0,0) B=(6,0,0) C=(0,6,0)
    // C = ((0+6+0)/3, (0+0+6)/3, (0+0+0)/3) = (2, 2, 0)
    TriangleData t = makeTriangle(0, 0, {0,0,0}, {6,0,0}, {0,6,0});
    Vec3 c = GeometryCalculator::centroid(t);
    EXPECT_TRUE(vec3NearlyEqual(c, {2.0, 2.0, 0.0}))
        << "centroide obtenido=(" << c.x << "," << c.y << "," << c.z << ")";
}

// --- Caso 2: triangulo equilatero en 3D -------------------------------------
TEST(Centroide, TrianguloEquilatero) {
    // Triangulo equilatero de lado 2 centrado en el origen, en el plano z=0.
    // Vertices clasicos: (1,0,0), (-0.5, sqrt(3)/2, 0), (-0.5, -sqrt(3)/2, 0)
    const double h = std::sqrt(3.0) / 2.0;
    TriangleData t = makeTriangle(0, 0, {1.0, 0.0, 0.0}, {-0.5, h, 0.0}, {-0.5, -h, 0.0});
    Vec3 c = GeometryCalculator::centroid(t);
    // Suma de x = 1 - 0.5 - 0.5 = 0 ; suma de y = 0 + h - h = 0
    EXPECT_TRUE(vec3NearlyEqual(c, {0.0, 0.0, 0.0}));
}

// --- Caso 3: triangulo degenerado (los 3 puntos colineales / coincidentes) -
TEST(Centroide, TrianguloDegeneradoColineal) {
    // A, B, C colineales sobre el eje X. El centroide sigue siendo el
    // promedio aritmetico (matematicamente valido), aunque el "triangulo"
    // tiene area 0 y no representa una superficie fisica real.
    TriangleData t = makeTriangle(0, 0, {0,0,0}, {2,0,0}, {4,0,0});
    Vec3 c = GeometryCalculator::centroid(t);
    EXPECT_TRUE(vec3NearlyEqual(c, {2.0, 0.0, 0.0}));
    // Se documenta explicitamente que el area debe ser 0 en este caso.
    EXPECT_NEAR(GeometryCalculator::area(t), 0.0, kEpsilon);
}

TEST(Centroide, TrianguloDegeneradoPuntosCoincidentes) {
    // Los 3 vertices son el mismo punto.
    TriangleData t = makeTriangle(0, 0, {3,3,3}, {3,3,3}, {3,3,3});
    Vec3 c = GeometryCalculator::centroid(t);
    EXPECT_TRUE(vec3NearlyEqual(c, {3.0, 3.0, 3.0}));
    EXPECT_NEAR(GeometryCalculator::area(t), 0.0, kEpsilon);
}

// --- Caso 4: coordenadas negativas ------------------------------------------
TEST(Centroide, CoordenadasNegativas) {
    // A=(-3,-3,-3) B=(-6,-3,-3) C=(-3,-6,-3)
    // C = ((-3-6-3)/3, (-3-3-6)/3, (-3-3-3)/3) = (-4, -4, -3)
    TriangleData t = makeTriangle(0, 0, {-3,-3,-3}, {-6,-3,-3}, {-3,-6,-3});
    Vec3 c = GeometryCalculator::centroid(t);
    EXPECT_TRUE(vec3NearlyEqual(c, {-4.0, -4.0, -3.0}));
}

// --- Caso 5: coordenadas grandes (orden de magnitud de una sala grande) ----
TEST(Centroide, CoordenadasGrandes) {
    // Valores en el orden de 10^5, para detectar perdida de precision.
    TriangleData t = makeTriangle(0, 0,
        {100000.0, 0.0, 0.0},
        {200000.0, 0.0, 0.0},
        {150000.0, 300000.0, 0.0});
    Vec3 c = GeometryCalculator::centroid(t);
    // C = (450000/3, 300000/3, 0) = (150000, 100000, 0)
    EXPECT_TRUE(vec3NearlyEqual(c, {150000.0, 100000.0, 0.0}, 1e-3));
}

// --- Caso adicional: el centroide del Escenario de Auditoria S1 ------------
TEST(Centroide, EscenarioS1_Piso) {
    Vec3 c = GeometryCalculator::centroid(T0_piso());
    EXPECT_TRUE(vec3NearlyEqual(c, {4.0/3.0, 4.0/3.0, 0.0}));
}

TEST(Centroide, EscenarioS1_Oeste) {
    Vec3 c = GeometryCalculator::centroid(T1_oeste());
    EXPECT_TRUE(vec3NearlyEqual(c, {0.0, 2.0/3.0, 2.0/3.0}));
}

TEST(Centroide, EscenarioS1_Sur) {
    Vec3 c = GeometryCalculator::centroid(T4_sur());
    EXPECT_TRUE(vec3NearlyEqual(c, {2.0, 0.0, 4.0/3.0}));
}

// El orden de los vertices NO debe afectar el centroide (a diferencia de la
// normal). Se verifica con T1 y su version de winding invertido T3.
TEST(Centroide, InvarianteAlOrdenDeVertices) {
    Vec3 c1 = GeometryCalculator::centroid(T1_oeste());
    Vec3 c3 = GeometryCalculator::centroid(T3_oeste_rev());
    EXPECT_TRUE(vec3NearlyEqual(c1, c3));
}
