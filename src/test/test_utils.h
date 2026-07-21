#pragma once
//Contiene:
//- Un generador de triangulos (makeTriangle) para no repetir codigo.
//- El "Escenario de Auditoria S1": un conjunto pequeno y controlado de
//triangulos, disenado especificamente para poder calcular a mano
//(con lapiz y papel) cada centroide, area, distancia, tiempo de vuelo,
//condicion de visibilidad y porcentaje de energia, y asi poder comparar
//el resultado EXACTO del programa contra el resultado teorico esperado.
//- Helpers de comparacion con tolerancia (para floats/doubles).

#include "SimulationTypes.h"
#include <cmath>
#include <vector>

namespace test_utils {

using core::Vec3;
using core::TriangleData;
using core::SurfaceData;
using core::SourceData;
using core::ReceiverData;
using core::ScenarioData;
using core::SimulationConfig;

constexpr double kEpsilon = 1e-6;

inline bool nearlyEqual(double a, double b, double eps = kEpsilon) {
    return std::fabs(a - b) <= eps;
}

inline bool vec3NearlyEqual(const Vec3& a, const Vec3& b, double eps = kEpsilon) {
    return nearlyEqual(a.x, b.x, eps) &&
           nearlyEqual(a.y, b.y, eps) &&
           nearlyEqual(a.z, b.z, eps);
}

inline TriangleData makeTriangle(int id, int surfaceId, Vec3 a, Vec3 b, Vec3 c) {
    TriangleData t;
    t.id = id;
    t.surfaceId = surfaceId;
    t.a = a;
    t.b = b;
    t.c = c;
    return t;
}

inline TriangleData T0_piso()      { return makeTriangle(0, 0, {0,0,0}, {4,0,0}, {0,4,0}); }
inline TriangleData T1_oeste()     { return makeTriangle(1, 1, {0,0,0}, {0,2,0}, {0,0,2}); }
inline TriangleData T2_techo()     { return makeTriangle(2, 2, {0,0,5}, {4,0,5}, {0,4,5}); }
inline TriangleData T3_oeste_rev() { return makeTriangle(3, 1, {0,0,0}, {0,0,2}, {0,2,0}); }
inline TriangleData T4_sur()       { return makeTriangle(4, 3, {0,0,0}, {0,0,4}, {6,0,0}); }
inline TriangleData T5_lejano()    { return makeTriangle(5, 4, {34,0,0}, {34,0,2}, {34,2,0}); }

inline ScenarioData buildScenarioS1(double absorption = 0.1) {
    ScenarioData scenario;

    SurfaceData floorSurf;
    floorSurf.id = 0; floorSurf.absorption = absorption;
    floorSurf.triangles = {T0_piso()};

    SurfaceData westSurf;
    westSurf.id = 1; westSurf.absorption = absorption;
    westSurf.triangles = {T1_oeste()};

    SurfaceData ceilSurf;
    ceilSurf.id = 2; ceilSurf.absorption = absorption;
    ceilSurf.triangles = {T2_techo()};

    SurfaceData southSurf;
    southSurf.id = 3; southSurf.absorption = absorption;
    southSurf.triangles = {T4_sur()};

    scenario.surfaces = {floorSurf, westSurf, ceilSurf, southSurf};

    SourceData src;
    src.id = 0;
    src.position = {1.0, 1.0, 0.5};
    src.energy = 1.0;
    scenario.sources = {src};

    ReceiverData recv;
    recv.id = 0;
    recv.position = {1.0, 1.0, 1.0};
    recv.radius = 0.5;
    scenario.receivers = {recv};

    return scenario;
}

inline SimulationConfig buildConfigS1(int durationMs = 200) {
    SimulationConfig cfg;
    cfg.durationMs = durationMs;
    cfg.soundSpeed = 340.0;
    cfg.rayCount = 64; 
    cfg.diffusionCoefficient = 0.5;
    return cfg;
}

} 