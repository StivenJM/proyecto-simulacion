#include <gtest/gtest.h>

#include "core/SurfaceTriangulator.h"

using namespace core;

TEST(SurfaceTriangulator, FourPointSurfaceBuildsNxNMeshWithTwoTrianglesPerCell) {
    SurfaceData surface;
    surface.id = 7;
    surface.absorption = 0.25;
    surface.outlinePoints = {
        {0.0, 0.0, 0.0},
        {2.0, 0.0, 0.0},
        {2.0, 2.0, 0.0},
        {0.0, 2.0, 0.0},
    };

    const std::vector<SurfaceData> result = SurfaceTriangulator::triangulateSurfaces({surface}, 2);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].triangles.size(), 8u);
    EXPECT_EQ(result[0].triangles.front().id, 1);
    EXPECT_EQ(result[0].triangles.back().id, 8);
    for (const TriangleData& triangle : result[0].triangles) {
        EXPECT_EQ(triangle.surfaceId, 7);
    }
}

TEST(SurfaceTriangulator, ClampsSubdivisionsToAtLeastOne) {
    SurfaceData surface;
    surface.id = 3;
    surface.outlinePoints = {
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {1.0, 1.0, 0.0},
        {0.0, 1.0, 0.0},
    };

    const std::vector<SurfaceData> result = SurfaceTriangulator::triangulateSurfaces({surface}, 0);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].triangles.size(), 2u);
}

TEST(SurfaceTriangulator, NonQuadOutlineUsesFanTriangulation) {
    SurfaceData surface;
    surface.id = 5;
    surface.outlinePoints = {
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {1.0, 1.0, 0.0},
        {0.5, 1.5, 0.0},
        {0.0, 1.0, 0.0},
    };

    const std::vector<SurfaceData> result = SurfaceTriangulator::triangulateSurfaces({surface}, 4);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].triangles.size(), 3u);
    for (const TriangleData& triangle : result[0].triangles) {
        EXPECT_EQ(triangle.surfaceId, 5);
    }
}

TEST(SurfaceTriangulator, KeepsLegacyTrianglesWhenOutlineIsMissing) {
    SurfaceData surface;
    surface.id = 9;
    surface.triangles = {{42, 0, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}}};

    const std::vector<SurfaceData> result = SurfaceTriangulator::triangulateSurfaces({surface}, 3);

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].triangles.size(), 1u);
    EXPECT_EQ(result[0].triangles[0].id, 42);
    EXPECT_EQ(result[0].triangles[0].surfaceId, 9);
}
