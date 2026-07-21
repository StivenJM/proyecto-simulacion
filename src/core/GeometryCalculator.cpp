#include "GeometryCalculator.h"
#include <cmath>
#include <algorithm>

namespace core {

namespace {

Vec3 subtract(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 scale(const Vec3& v, double factor) {
    return {v.x * factor, v.y * factor, v.z * factor};
}

double length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

Vec3 normalize(const Vec3& v) {
    double l = length(v);
    if (l < 1e-10) return {0.0, 0.0, 0.0};
    return {v.x / l, v.y / l, v.z / l};
}

double signedPlaneDistance(const Vec3& point, const Vec3& planePoint, const Vec3& planeNormal) {
    return dot(subtract(point, planePoint), planeNormal);
}

bool areCoplanar(const TriangleData& a, const TriangleData& b) {
    Vec3 na = GeometryCalculator::normalVector(a);
    Vec3 nb = GeometryCalculator::normalVector(b);
    if (length(na) < 1e-10 || length(nb) < 1e-10) return true;
    if (std::abs(dot(na, nb)) <= 0.99) return false;

    constexpr double planeEpsilon = 1e-6;
    return std::abs(signedPlaneDistance(b.a, a.a, na)) <= planeEpsilon &&
           std::abs(signedPlaneDistance(b.b, a.a, na)) <= planeEpsilon &&
           std::abs(signedPlaneDistance(b.c, a.a, na)) <= planeEpsilon;
}

double solidAngleWeight(const TriangleData& src, const TriangleData& dst) {
    Vec3 cSrc = GeometryCalculator::centroid(src);
    Vec3 cDst = GeometryCalculator::centroid(dst);
    const double d = GeometryCalculator::distance(cSrc, cDst);
    if (d < 1e-10) return 0.0;

    Vec3 dir = normalize(subtract(cDst, cSrc));
    Vec3 nSrc = GeometryCalculator::normalVector(src);
    Vec3 nDst = GeometryCalculator::normalVector(dst);

    const double cosI = std::abs(dot(nSrc, dir));
    const Vec3 negDir = scale(dir, -1.0);
    const double cosJ = std::abs(dot(nDst, negDir));
    const double areaDst = GeometryCalculator::area(dst);

    constexpr double pi = 3.14159265358979323846;
    return (cosI * cosJ * areaDst) / (pi * d * d);
}

} // namespace

Vec3 GeometryCalculator::centroid(const TriangleData& tri) {
    return {
        (tri.a.x + tri.b.x + tri.c.x) / 3.0,
        (tri.a.y + tri.b.y + tri.c.y) / 3.0,
        (tri.a.z + tri.b.z + tri.c.z) / 3.0
    };
}

double GeometryCalculator::area(const TriangleData& tri) {
    Vec3 ab = subtract(tri.b, tri.a);
    Vec3 ac = subtract(tri.c, tri.a);
    return length(cross(ab, ac)) / 2.0;
}

Vec3 GeometryCalculator::normalVector(const TriangleData& tri) {
    Vec3 ab = subtract(tri.b, tri.a);
    Vec3 ac = subtract(tri.c, tri.a);
    return normalize(cross(ab, ac));
}

double GeometryCalculator::distance(const Vec3& a, const Vec3& b) {
    return length(subtract(b, a));
}

int GeometryCalculator::timeOfFlightMs(double distanceMeters, double soundSpeed) {
    if (soundSpeed <= 0.0) return 0;
    return static_cast<int>(std::lround((distanceMeters / soundSpeed) * 1000.0));
}

bool GeometryCalculator::areVisible(const TriangleData& a, const TriangleData& b) {
    // Triangulos coplanares no son visibles entre si (RF-04)
    return !areCoplanar(a, b);
}

DiffusionMatrixData GeometryCalculator::buildDiffusionMatrix(
    const std::vector<TriangleData>& triangles,
    double soundSpeed
) {
    int n = static_cast<int>(triangles.size());
    DiffusionMatrixData result;

    result.distances.assign(n, std::vector<double>(n, 0.0));
    result.timesMs.assign(n, std::vector<int>(n, 0));
    result.percentages.assign(n, std::vector<double>(n, 0.0));
    result.visibility.assign(n, std::vector<bool>(n, false));

    for (int i = 0; i < n; i++) {
        Vec3 ci = centroid(triangles[i]);
        std::vector<double> weights(n, 0.0);
        double totalWeight = 0.0;

        for (int j = 0; j < n; j++) {
            if (i == j) continue;

            Vec3   cj   = centroid(triangles[j]);
            double dist = distance(ci, cj);

            result.distances[i][j]  = dist;
            result.timesMs[i][j]    = timeOfFlightMs(dist, soundSpeed);
            result.visibility[i][j] = areVisible(triangles[i], triangles[j]);

            if (result.visibility[i][j]) {
                weights[j] = solidAngleWeight(triangles[i], triangles[j]);
                totalWeight += weights[j];
            }
        }

        // RF-06: distribucion ponderada por angulo solido entre triangulos visibles.
        if (totalWeight > 1e-12) {
            for (int j = 0; j < n; j++) {
                if (i != j && result.visibility[i][j]) {
                    result.percentages[i][j] = weights[j] / totalWeight;
                }
            }
        }
    }

    return result;
}

} // namespace core
