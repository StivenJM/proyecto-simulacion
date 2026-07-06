#include "RayTracer.h"
#include "GeometryCalculator.h"
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
    std::vector<Vec3> dirs;
    dirs.reserve(count);
    const double golden = (1.0 + std::sqrt(5.0)) / 2.0;
    for (int i = 0; i < count; i++) {
        double theta = 2.0 * PI * i / golden;
        double phi   = std::acos(1.0 - 2.0 * (i + 0.5) / count);
        dirs.push_back({std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta), std::cos(phi)});
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
    const double energyPerRay = source.energy / config.rayCount;
    const auto   rayDirs      = generateRayDirections(config.rayCount);

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

            // RF-08: aplicar absorcion y separar fraccion difusa
            double remaining = energy * (1.0 - allTris[nearestIdx].absorption);
            energy    = remaining * (1.0 - config.diffusionCoefficient);
            origin    = hitPoint;
            direction = reflect(direction, GeometryCalculator::normalVector(allTris[nearestIdx].tri));
        }
    }

    return output;
}

} // namespace core
