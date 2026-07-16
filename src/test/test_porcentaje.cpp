#include <gtest/gtest.h>
#include <cmath>
#include "services/implementations/core/GeometryCalculator.h"
#include "test_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace services;
using namespace services::core;
using namespace test_utils;

namespace {
// Recalcula, con la formula teorica de angulo solido 
double formFactorTeorico(const TriangleData& src, const TriangleData& dst) {
    Vec3 nSrc = GeometryCalculator::normalVector(src);
    Vec3 nDst = GeometryCalculator::normalVector(dst);
    Vec3 cSrc = GeometryCalculator::centroid(src);
    Vec3 cDst = GeometryCalculator::centroid(dst);
    double d = GeometryCalculator::distance(cSrc, cDst);

    Vec3 dir{(cDst.x - cSrc.x) / d, (cDst.y - cSrc.y) / d, (cDst.z - cSrc.z) / d};
    double cosI = nSrc.x*dir.x + nSrc.y*dir.y + nSrc.z*dir.z;
    double cosJ = -(nDst.x*dir.x + nDst.y*dir.y + nDst.z*dir.z);
    double aDst = GeometryCalculator::area(dst);

    return (cosI * cosJ * aDst) / (M_PI * d * d);
}
}

// --- Propiedad 1: la suma de porcentajes salientes de un triangulo = 100% --
TEST(Porcentaje, SumaDePorcentajesEsUno) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    for (size_t i = 0; i < tris.size(); i++) {
        double suma = 0.0;
        for (size_t j = 0; j < tris.size(); j++) suma += diff.percentages[i][j];
        // Solo tiene sentido exigir suma=1 si el triangulo tiene al menos un
        // vecino visible; en este escenario T0, T1 y T4 se ven mutuamente.
        EXPECT_NEAR(suma, 1.0, kEpsilon) << "fila i=" << i;
    }
}

// --- Propiedad 2: ningun porcentaje negativo --------------------------------
TEST(Porcentaje, NingunPorcentajeNegativo) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T2_techo(), T3_oeste_rev(), T4_sur(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    for (size_t i = 0; i < tris.size(); i++)
        for (size_t j = 0; j < tris.size(); j++)
            EXPECT_GE(diff.percentages[i][j], 0.0) << "[" << i << "][" << j << "]";
}

// --- Propiedad 3: ningun porcentaje mayor a 100% ----------------------------
TEST(Porcentaje, NingunPorcentajeMayorAUno) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T2_techo(), T3_oeste_rev(), T4_sur(), T5_lejano()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    for (size_t i = 0; i < tris.size(); i++)
        for (size_t j = 0; j < tris.size(); j++)
            EXPECT_LE(diff.percentages[i][j], 1.0 + kEpsilon) << "[" << i << "][" << j << "]";
}

TEST(Porcentaje, Bug_ComportamientoActual_RepartoUniforme) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    // indices: 0=T0, 1=T1, 2=T4
    EXPECT_NEAR(diff.percentages[0][1], 0.5, kEpsilon)
        << "Comportamiento actual: reparto uniforme (1/2) ignorando area y distancia";
    EXPECT_NEAR(diff.percentages[0][2], 0.5, kEpsilon)
        << "Comportamiento actual: reparto uniforme (1/2) ignorando area y distancia";
}

TEST(Porcentaje, Bug_FormFactorTeoricoDifiereMuchoDelUniforme) {
    double fT1 = formFactorTeorico(T0_piso(), T1_oeste());
    double fT4 = formFactorTeorico(T0_piso(), T4_sur());
    double suma = fT1 + fT4;
    double pctTeoricoT1 = fT1 / suma;
    double pctTeoricoT4 = fT4 / suma;

    EXPECT_NEAR(pctTeoricoT1, 0.157895, 1e-4);
    EXPECT_NEAR(pctTeoricoT4, 0.842105, 1e-4);

    // Error relativo del codigo actual (50%) respecto del valor teorico:
    double errRelT1 = std::fabs(0.5 - pctTeoricoT1) / pctTeoricoT1;
    double errRelT4 = std::fabs(0.5 - pctTeoricoT4) / pctTeoricoT4;
    EXPECT_GT(errRelT1, 2.0) << "Error relativo > 200% para el triangulo cercano/pequeno";
    EXPECT_GT(errRelT4, 0.30) << "Error relativo > 30% para el triangulo lejano/grande";
}

TEST(Porcentaje, DISABLED_EspecificacionCorrecta_PonderadoPorAnguloSolido) {
    // Especificacion deseada: el porcentaje debe reflejar el angulo solido
    // real (area/distancia/orientacion), no un conteo uniforme de vecinos.
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste(), T4_sur()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    EXPECT_NEAR(diff.percentages[0][1], 0.157895, 1e-3);
    EXPECT_NEAR(diff.percentages[0][2], 0.842105, 1e-3);
}

TEST(Porcentaje, Bug_CambioDeWindingEnUnTrianguloCorrompeOtrasFilas) {
    std::vector<TriangleData> conT1 = {T0_piso(), T1_oeste(), T4_sur()};
    auto diffConT1 = GeometryCalculator::buildDiffusionMatrix(conT1, 340.0);
    // indices: 0=T0, 1=T1, 2=T4  -> percentages[2][0] es T4->T0
    EXPECT_NEAR(diffConT1.percentages[2][0], 0.5, kEpsilon);

    std::vector<TriangleData> conT3 = {T0_piso(), T3_oeste_rev(), T4_sur()};
    auto diffConT3 = GeometryCalculator::buildDiffusionMatrix(conT3, 340.0);
    // indices: 0=T0, 1=T3, 2=T4  -> percentages[2][0] es T4->T0
    EXPECT_NEAR(diffConT3.percentages[2][0], 1.0, kEpsilon)
        << "Al invertir el winding de un triangulo ajeno (T1->T3), la "
           "distribucion de energia de T4 hacia T0 cambia de 50% a 100%, "
           "sin que la geometria real de T4 o T0 se haya modificado.";
}
