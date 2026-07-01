#pragma once
#include "services/SimulationTypes.h"
#include <vector>

namespace services {
namespace core {

class GeometryCalculator {
public:
    static Vec3   centroid(const TriangleData& tri);
    static double area(const TriangleData& tri);
    static Vec3   normalVector(const TriangleData& tri);
    static double distance(const Vec3& a, const Vec3& b);
    static int    timeOfFlightMs(double distanceMeters, double soundSpeed);
    static bool   areVisible(const TriangleData& a, const TriangleData& b);

    static DiffusionMatrixData buildDiffusionMatrix(
        const std::vector<TriangleData>& triangles,
        double soundSpeed
    );
};

} // namespace core
} // namespace services
