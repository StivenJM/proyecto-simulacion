#pragma once
#include "SimulationTypes.h"
#include <vector>

namespace core {

class GeometryCalculator {
public:
    // RF-02: Calcula el centroide (punto central) de un triangulo.
    static Vec3 centroid(const TriangleData& tri);

    // RF-03: Calcula el area de un triangulo usando producto vectorial.
    static double area(const TriangleData& tri);

    // RF-03: Retorna el vector normal unitario perpendicular al plano del triangulo.
    static Vec3 normalVector(const TriangleData& tri);

    // RF-03: Calcula la distancia euclidiana entre dos puntos en 3D.
    static double distance(const Vec3& a, const Vec3& b);

    // RF-05: Calcula el tiempo de vuelo en milisegundos dado distancia y velocidad del sonido.
    static int timeOfFlightMs(double distanceMeters, double soundSpeed);

    // RF-04: Determina si dos triangulos se pueden ver entre si.
    // Dos triangulos no son visibles entre si si pertenecen al mismo plano.
    static bool areVisible(const TriangleData& a, const TriangleData& b);

    // RF-03, RF-04, RF-05, RF-06: Construye la matriz completa de difusion:
    // distancias entre centroides, tiempos de vuelo, visibilidad y porcentajes de distribucion.
    static DiffusionMatrixData buildDiffusionMatrix(
        const std::vector<TriangleData>& triangles,
        double soundSpeed
    );
};

} // namespace core
