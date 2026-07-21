#include "RayTracer.h"
#include "GeometryCalculator.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace core {

namespace {

const double PI             = 3.14159265358979323846;
const double ENERGY_MINIMUM = 0.001;

Vec3 subtract(const Vec3& a, const Vec3& b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
Vec3 add(const Vec3& a, const Vec3& b)      { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 scale(const Vec3& v, double s)         { return {v.x*s, v.y*s, v.z*s}; }

double dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

double length(const Vec3& v) { return std::sqrt(dot(v, v)); }

Vec3 normalize(const Vec3& v) {
    double l = length(v);
    if (l < 1e-10) return {0.0, 0.0, 0.0};
    return {v.x/l, v.y/l, v.z/l};
}

Vec3 sphericalInterpolate(const Vec3& from, const Vec3& to, double amount) {
    const double clampedDot = std::max(-1.0, std::min(1.0, dot(from, to)));
    const double angle = std::acos(clampedDot);
    if (angle < 1e-10) return from;

    const double sinAngle = std::sin(angle);
    const double fromWeight = std::sin((1.0 - amount) * angle) / sinAngle;
    const double toWeight = std::sin(amount * angle) / sinAngle;
    return normalize(add(scale(from, fromWeight), scale(to, toWeight)));
}

} // namespace

double RayTracer::intersectRayTriangle(
    const Vec3& origin, const Vec3& direction, const TriangleData& tri
) {
    const double EPS = 1e-8;
    Vec3   ab = subtract(tri.b, tri.a);
    Vec3   ac = subtract(tri.c, tri.a);
    Vec3   h  = cross(direction, ac);
    double a  = dot(ab, h);

    if (a > -EPS && a < EPS) return -1.0;

    double f = 1.0 / a;
    Vec3   s = subtract(origin, tri.a);
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return -1.0;

    Vec3   q = cross(s, ab);
    double v = f * dot(direction, q);
    if (v < 0.0 || u + v > 1.0) return -1.0;

    double t = f * dot(ac, q);
    return (t > EPS) ? t : -1.0;
}

Vec3 RayTracer::reflect(const Vec3& direction, const Vec3& normal) {
    double d = 2.0 * dot(direction, normal);
    return normalize({direction.x - d*normal.x, direction.y - d*normal.y, direction.z - d*normal.z});
}

std::vector<Vec3> RayTracer::generateRayDirections(int count) {
    const std::array<std::array<int, 3>, 30> edges{{
        {{0, 1, 0}}, {{0, 2, 0}}, {{0, 3, 0}}, {{0, 4, 0}}, {{0, 5, 0}},
        {{1, 6, 0}}, {{2, 6, 0}}, {{2, 7, 0}}, {{3, 7, 0}}, {{3, 8, 0}},
        {{4, 8, 0}}, {{4, 9, 0}}, {{5, 9, 0}}, {{5, 10, 0}}, {{1, 10, 0}},
        {{6, 11, 0}}, {{7, 11, 0}}, {{8, 11, 0}}, {{9, 11, 0}}, {{10, 11, 0}},
        {{1, 2, 0}}, {{2, 3, 0}}, {{3, 4, 0}}, {{4, 5, 0}}, {{5, 1, 0}},
        {{6, 7, 0}}, {{7, 8, 0}}, {{8, 9, 0}}, {{9, 10, 0}}, {{10, 6, 0}}
    }};

    const std::array<std::array<int, 3>, 20> triangles{{
        {{0, 1, 0}}, {{1, 2, 0}}, {{2, 3, 0}}, {{3, 4, 0}}, {{4, 0, 0}},
        {{5, 6, -1}}, {{6, 7, 0}}, {{7, 8, -1}}, {{8, 9, 0}}, {{9, 10, -1}},
        {{10, 11, 0}}, {{11, 12, -1}}, {{12, 13, 0}}, {{13, 14, -1}}, {{14, 5, 0}},
        {{15, 16, -1}}, {{16, 17, -1}}, {{17, 18, -1}}, {{18, 19, -1}}, {{19, 15, -1}}
    }};

    const double requested = std::max(5, count);
    const int n = std::max(1, static_cast<int>(std::floor(std::sqrt((requested - 2.0) / 10.0) + 0.5)));
    const int generatedCount = 2 + 10 * n * n;

    std::vector<Vec3> dirs;
    dirs.reserve(generatedCount);

    const double s = 2.0 / std::sqrt(5.0);
    const double r = (5.0 - std::sqrt(5.0)) / 5.0;

    dirs.push_back({0.0, 0.0, 1.0});
    for (int i = 1; i < 6; ++i) {
        const double upperAngle = PI * i * 72.0 / 180.0;
        dirs.push_back({s * std::cos(upperAngle), s * std::sin(upperAngle), 1.0 - r});
    }
    for (int i = 1; i < 6; ++i) {
        const double lowerAngle = 72.0 * PI * i / 180.0 + 36.0 * PI / 180.0;
        dirs.push_back({s * std::cos(lowerAngle), s * std::sin(lowerAngle), r - 1.0});
    }
    dirs.push_back({0.0, 0.0, -1.0});

    std::array<int, 30> edgeStartIndex{};
    for (std::size_t edgeIndex = 0; edgeIndex < edges.size(); ++edgeIndex) {
        edgeStartIndex[edgeIndex] = static_cast<int>(dirs.size());
        const Vec3 from = dirs[edges[edgeIndex][0]];
        const Vec3 to = dirs[edges[edgeIndex][1]];
        for (int i = 1; i < n; ++i) {
            dirs.push_back(sphericalInterpolate(from, to, static_cast<double>(i) / static_cast<double>(n)));
        }
    }

    for (const auto& triangle : triangles) {
        for (int j = 1; j < n; ++j) {
            const Vec3 from = dirs[edgeStartIndex[triangle[0]] + j - 1];
            const Vec3 to = dirs[edgeStartIndex[triangle[1]] + j - 1];
            const int subdivisions = triangle[2] == 0 ? j : n - j;
            for (int i = 1; i < subdivisions; ++i) {
                dirs.push_back(sphericalInterpolate(from, to, static_cast<double>(i) / static_cast<double>(subdivisions)));
            }
        }
    }

    return dirs;
}

RayTracer::TraceOutput RayTracer::trace(
    const SourceData&                source,
    const std::vector<SurfaceData>&  surfaces,
    const std::vector<ReceiverData>& receivers,
    const SimulationConfig&          config
) {
    TraceOutput output;

    struct FlatTri { TriangleData tri; double absorption; };
    std::vector<FlatTri> allTris;
    for (const auto& surf : surfaces)
        for (const auto& tri : surf.triangles)
            allTris.push_back({tri, surf.absorption});

    if (allTris.empty()) return output;

    const double maxDist      = config.soundSpeed * config.durationMs / 1000.0;
    const auto   rayDirs      = generateRayDirections(config.rayCount);
    const double energyPerRay = source.energy / static_cast<double>(rayDirs.size());

    for (const Vec3& rayDir : rayDirs) {
        Vec3   origin    = source.position;
        Vec3   direction = rayDir;
        double energy    = energyPerRay;
        double traveled  = 0.0;

        while (energy > ENERGY_MINIMUM && traveled < maxDist) {
            double nearestT   = -1.0;
            int    nearestIdx = -1;

            for (int i = 0; i < static_cast<int>(allTris.size()); i++) {
                double t = intersectRayTriangle(origin, direction, allTris[i].tri);
                if (t > 0.0 && (nearestT < 0.0 || t < nearestT)) {
                    nearestT = t; nearestIdx = i;
                }
            }

            if (nearestIdx < 0) break;

            Vec3 hitPoint = add(origin, scale(direction, nearestT));
            traveled     += nearestT;
            int timeMs    = static_cast<int>((traveled / config.soundSpeed) * 1000.0);

            // RF-09: detectar receptores a lo largo del segmento
            for (const auto& recv : receivers) {
                Vec3   op   = subtract(recv.position, origin);
                double t    = dot(op, direction);
                if (t < 0.0 || t > nearestT) continue;
                Vec3   cl   = add(origin, scale(direction, t));
                double dist = length(subtract(recv.position, cl));
                if (dist <= recv.radius) {
                    double recvTraveled = traveled - nearestT + t;
                    int    recvTimeMs   = static_cast<int>((recvTraveled / config.soundSpeed) * 1000.0);
                    if (recvTimeMs <= config.durationMs)
                        output.receiverEnergy.push_back({recv.id, recvTimeMs, energy});
                }
            }

            // Guardar segmento de rayo (RF-11)
            if (timeMs <= config.durationMs)
                output.rays.push_back({origin, hitPoint, energy, timeMs});

            // RF-08: aplicar absorcion del plano impactado y separar la energia difusa sembrada.
            const double remainingEnergy = energy * (1.0 - allTris[nearestIdx].absorption);
            const double diffuseEnergy = remainingEnergy * config.diffusionCoefficient;
            if (timeMs <= config.durationMs && diffuseEnergy > 0.0) {
                const TriangleData& hitTriangle = allTris[nearestIdx].tri;
                output.diffuseSeeds.push_back({
                    hitTriangle.id,
                    hitTriangle.surfaceId,
                    timeMs,
                    diffuseEnergy
                });
            }

            energy = remainingEnergy - diffuseEnergy;
            origin    = hitPoint;
            direction = reflect(direction, GeometryCalculator::normalVector(allTris[nearestIdx].tri));
        }
    }

    return output;
}

} // namespace core
