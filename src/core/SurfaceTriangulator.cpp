#include "SurfaceTriangulator.h"

#include <algorithm>

namespace core {
namespace {

Vec3 bilinear(const Vec3& p00, const Vec3& p10, const Vec3& p11, const Vec3& p01, double u, double v)
{
    const double w00 = (1.0 - u) * (1.0 - v);
    const double w10 = u * (1.0 - v);
    const double w11 = u * v;
    const double w01 = (1.0 - u) * v;

    return {
        p00.x * w00 + p10.x * w10 + p11.x * w11 + p01.x * w01,
        p00.y * w00 + p10.y * w10 + p11.y * w11 + p01.y * w01,
        p00.z * w00 + p10.z * w10 + p11.z * w11 + p01.z * w01,
    };
}

TriangleData makeTriangle(int id, int surfaceId, Vec3 a, Vec3 b, Vec3 c)
{
    return {id, surfaceId, a, b, c};
}

} // namespace

std::vector<SurfaceData> SurfaceTriangulator::triangulateSurfaces(
    const std::vector<SurfaceData>& surfaces,
    int meshSubdivisions
) {
    const int clampedSubdivisions = std::max(1, meshSubdivisions);
    int nextTriangleId = 1;

    std::vector<SurfaceData> triangulated;
    triangulated.reserve(surfaces.size());

    for (const SurfaceData& surface : surfaces) {
        SurfaceData mapped = triangulateSurface(surface, clampedSubdivisions, nextTriangleId);
        if (!mapped.triangles.empty()) {
            triangulated.push_back(std::move(mapped));
        }
    }

    return triangulated;
}

SurfaceData SurfaceTriangulator::triangulateSurface(
    const SurfaceData& surface,
    int meshSubdivisions,
    int& nextTriangleId
) {
    SurfaceData mapped = surface;
    mapped.triangles.clear();

    if (surface.outlinePoints.size() == 4) {
        const Vec3 p00 = surface.outlinePoints[0];
        const Vec3 p10 = surface.outlinePoints[1];
        const Vec3 p11 = surface.outlinePoints[2];
        const Vec3 p01 = surface.outlinePoints[3];

        mapped.triangles.reserve(static_cast<std::size_t>(meshSubdivisions * meshSubdivisions * 2));
        for (int j = 0; j < meshSubdivisions; ++j) {
            const double v0 = static_cast<double>(j) / static_cast<double>(meshSubdivisions);
            const double v1 = static_cast<double>(j + 1) / static_cast<double>(meshSubdivisions);
            for (int i = 0; i < meshSubdivisions; ++i) {
                const double u0 = static_cast<double>(i) / static_cast<double>(meshSubdivisions);
                const double u1 = static_cast<double>(i + 1) / static_cast<double>(meshSubdivisions);

                const Vec3 bottomLeft = bilinear(p00, p10, p11, p01, u0, v0);
                const Vec3 bottomRight = bilinear(p00, p10, p11, p01, u1, v0);
                const Vec3 topLeft = bilinear(p00, p10, p11, p01, u0, v1);
                const Vec3 topRight = bilinear(p00, p10, p11, p01, u1, v1);

                mapped.triangles.push_back(makeTriangle(nextTriangleId++, surface.id, bottomLeft, bottomRight, topRight));
                mapped.triangles.push_back(makeTriangle(nextTriangleId++, surface.id, bottomLeft, topRight, topLeft));
            }
        }
        return mapped;
    }

    if (surface.outlinePoints.size() >= 3) {
        mapped.triangles.reserve(surface.outlinePoints.size() - 2);
        const Vec3 origin = surface.outlinePoints.front();
        for (std::size_t index = 1; index + 1 < surface.outlinePoints.size(); ++index) {
            mapped.triangles.push_back(makeTriangle(
                nextTriangleId++,
                surface.id,
                origin,
                surface.outlinePoints[index],
                surface.outlinePoints[index + 1]
            ));
        }
        return mapped;
    }

    mapped.triangles = surface.triangles;
    for (TriangleData& triangle : mapped.triangles) {
        triangle.surfaceId = surface.id;
        if (triangle.id <= 0) {
            triangle.id = nextTriangleId++;
        } else {
            nextTriangleId = std::max(nextTriangleId, triangle.id + 1);
        }
    }
    return mapped;
}

} // namespace core
