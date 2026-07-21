#include <gtest/gtest.h>
#include "core/GeometryCalculator.h"
#include "test_utils.h"

using namespace core;
using namespace test_utils;

// --- Caso exacto: sin resto, no hay ambiguedad entre truncar/redondear -----
TEST(Tiempo, DistanciaExacta_SinResto) {
    // d=34m, V=340 m/s -> t = 34/340*1000 = 100.0 ms exactos
    int t = GeometryCalculator::timeOfFlightMs(34.0, 340.0);
    EXPECT_EQ(t, 100);
}

TEST(Tiempo, DistanciaCero) {
    int t = GeometryCalculator::timeOfFlightMs(0.0, 340.0);
    EXPECT_EQ(t, 0);
}

// --- Contrato actual: redondeo al milisegundo mas cercano -------------------
TEST(Tiempo, EspecificacionCorrecta_RedondeoPisoOeste) {
    // dist(T0,T1) = sqrt(8/3) = 1.632993... m
    // t_exacto = 1.632993/340*1000 = 4.802921 ms
    double d = std::sqrt(8.0 / 3.0);
    int t = GeometryCalculator::timeOfFlightMs(d, 340.0);
    EXPECT_EQ(t, 5) << "4.802921 ms debe redondear a 5, no truncar a 4";
}

TEST(Tiempo, EspecificacionCorrecta_RedondeoPisoSur) {
    // dist(T0,T4) = 2.0 m exactos
    // t_exacto = 2.0/340*1000 = 5.882353 ms
    int t = GeometryCalculator::timeOfFlightMs(2.0, 340.0);
    EXPECT_EQ(t, 6) << "5.882353 ms debe redondear a 6, no truncar a 5";
}

TEST(Tiempo, Precision_ValorFraccionarioAltoRedondeaArriba) {
    // d tal que t_exacto = 9.999 ms; debe redondear a 10.
    double d = 9.999 / 1000.0 * 340.0;
    int t = GeometryCalculator::timeOfFlightMs(d, 340.0);
    EXPECT_EQ(t, 10) << "9.999 ms debe redondear a 10";
}

// --- Velocidad de sonido invalida (caso limite defensivo) ------------------
TEST(Tiempo, VelocidadSonidoCeroOMenor_NoDivPorCero) {
    EXPECT_EQ(GeometryCalculator::timeOfFlightMs(10.0, 0.0), 0);
    EXPECT_EQ(GeometryCalculator::timeOfFlightMs(10.0, -340.0), 0);
}

// --- La matriz "tiempo" (timesMs) usa V_SON=340 tal como exige el enunciado -
TEST(Tiempo, MatrizUsaVelocidadDeSonidoDelParametro) {
    std::vector<TriangleData> tris = {T0_piso(), T1_oeste()};
    auto diff = GeometryCalculator::buildDiffusionMatrix(tris, 340.0);
    // recalculamos el valor esperado con la misma formula que exige el
    // enunciado, y confirmamos que coincide con lo que guarda la matriz.
    double d = diff.distances[0][1];
    int esperado = static_cast<int>(std::lround((d / 340.0) * 1000.0));
    EXPECT_EQ(diff.timesMs[0][1], esperado);
}
