#include <gtest/gtest.h>
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Mismo plano => NO visible (caso simple, sin ambiguedad de winding) ----
TEST(Visibilidad, MismoPlano_NoVisible) {
    // Dos triangulos coplanares en z=0, mismo winding (misma normal).
    TriangleData a = makeTriangle(0, 0, {0,0,0}, {4,0,0}, {0,4,0});
    TriangleData b = makeTriangle(1, 0, {10,0,0}, {14,0,0}, {10,4,0});
    EXPECT_FALSE(GeometryCalculator::areVisible(a, b));
}

TEST(Visibilidad, MismoPlano_WindingOpuesto_SigueSiendoNoVisible) {
    TriangleData a = makeTriangle(0, 0, {0,0,0}, {4,0,0}, {0,4,0});
    TriangleData b = makeTriangle(1, 0, {10,0,0}, {10,4,0}, {14,0,0}); // orden invertido
    EXPECT_FALSE(GeometryCalculator::areVisible(a, b));
}

// --- Planos distintos, correctamente orientados => visible -----------------
TEST(Visibilidad, PlanosDistintos_Visible) {
    TriangleData piso  = T0_piso();
    TriangleData oeste = T1_oeste();
    EXPECT_TRUE(GeometryCalculator::areVisible(piso, oeste));
}

TEST(Visibilidad, PlanosDistintos_Visible_PisoSur) {
    EXPECT_TRUE(GeometryCalculator::areVisible(T0_piso(), T4_sur()));
}

TEST(Visibilidad, PlanosParalelosDistintosSonVisibles) {
    bool visible = GeometryCalculator::areVisible(T0_piso(), T2_techo());
    EXPECT_TRUE(visible)
        << "Solo los triangulos coplanares deben ser no visibles; planos paralelos separados siguen siendo visibles.";
}

TEST(Visibilidad, Bug_MismaGeometria_MismosPuntos) {
    // Verificacion previa: T1 y T3 son geometricamente el mismo triangulo
    // (mismo centroide, misma area), solo cambia el orden de sus vertices.
    Vec3 c1 = GeometryCalculator::centroid(T1_oeste());
    Vec3 c3 = GeometryCalculator::centroid(T3_oeste_rev());
    ASSERT_TRUE(vec3NearlyEqual(c1, c3));
    ASSERT_NEAR(GeometryCalculator::area(T1_oeste()), GeometryCalculator::area(T3_oeste_rev()), kEpsilon);
}

TEST(Visibilidad, EspecificacionCorrecta_IndependienteDelWinding) {
    bool visibleConT1 = GeometryCalculator::areVisible(T0_piso(), T1_oeste());
    bool visibleConT3 = GeometryCalculator::areVisible(T0_piso(), T3_oeste_rev());

    EXPECT_TRUE(visibleConT1)
        << "T1 debe ser visible desde el piso";
    EXPECT_TRUE(visibleConT3)
        << "T3 representa el mismo triangulo fisico con winding invertido y tambien debe ser visible";
    EXPECT_EQ(visibleConT1, visibleConT3)
        << "La visibilidad no deberia depender del orden de los vertices, "
            "pero en este codigo lo hace.";
}

// --- Caso limite: triangulos casi coplanares (bajo el umbral 0.99) ---------
TEST(Visibilidad, CasoLimite_CercaDelUmbralDeCoplanaridad) {
    TriangleData a = makeTriangle(0, 0, {0,0,0}, {4,0,0}, {0,4,0}); // normal (0,0,1)
    TriangleData b = makeTriangle(1, 0, {10,0,0}, {14,0,0.35}, {10,4,0});
    double dotNormals = std::fabs(
        GeometryCalculator::normalVector(a).x * GeometryCalculator::normalVector(b).x +
        GeometryCalculator::normalVector(a).y * GeometryCalculator::normalVector(b).y +
        GeometryCalculator::normalVector(a).z * GeometryCalculator::normalVector(b).z);
    EXPECT_GT(dotNormals, 0.9);
}
