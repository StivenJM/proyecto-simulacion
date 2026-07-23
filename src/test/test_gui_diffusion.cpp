#include <gtest/gtest.h>

#include "core/ISimulationService.h"
#include "gui/services/implementations/core/CoreSimulationService.h"
#include "gui/services/implementations/core/mappers/ResultMapper.h"

#include <memory>

using gui::CoreSimulationService;
using gui::GuiScenario;
using gui::SimulationResultDto;

namespace {

class CapturingCoreService final : public core::ISimulationService {
public:
    core::SimulationResult runSimulation(
        const core::ScenarioData& scenario,
        const core::SimulationConfig& config
    ) override {
        lastScenario = scenario;
        lastConfig = config;
        core::SimulationResult result;
        result.success = true;
        result.message = "ok";
        return result;
    }

    core::ScenarioData lastScenario;
    core::SimulationConfig lastConfig;
};

GuiScenario minimalScenario()
{
    GuiScenario scenario;
    gui::GuiPlane plane;
    plane.id = 7;
    plane.name = "Panel";
    plane.outlinePoints = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    scenario.planes.push_back(plane);

    gui::GuiSource source;
    source.id = 3;
    source.position = {0.0f, 0.0f, 1.0f};
    source.energy = 1.0f;
    scenario.sources.push_back(source);
    return scenario;
}

}  // namespace

TEST(GuiDiffusion, ResultDtoExponeMatricesCompletasYEnergiaDensa) {
    core::SimulationResult coreResult;
    coreResult.success = true;
    coreResult.message = "ok";
    coreResult.diffusion.distances = {{0.0, 2.0}, {2.0, 0.0}};
    coreResult.diffusion.timesMs = {{0, 6}, {6, 0}};
    coreResult.diffusion.percentages = {{0.0, 1.0}, {1.0, 0.0}};
    coreResult.diffusion.visibility = {{false, true}, {true, false}};
    coreResult.diffusionTriangles = {
        {10, 7, {}, {}, {}},
        {11, 8, {}, {}, {}}
    };
    coreResult.diffuseEnergyByTriangleTime = {{1.25, 0.0, 0.5}, {0.0, 2.0, 0.0}};

    GuiScenario scenario = minimalScenario();
    const SimulationResultDto dto = gui::coremappers::toGuiResult(coreResult, scenario);

    ASSERT_EQ(dto.diffusion.triangles.size(), 2u);
    EXPECT_EQ(dto.diffusion.triangles[0].triangleId, 10);
    EXPECT_EQ(dto.diffusion.triangles[0].planeId, 7);
    EXPECT_EQ(dto.diffusion.distances, coreResult.diffusion.distances);
    EXPECT_EQ(dto.diffusion.timesMs, coreResult.diffusion.timesMs);
    EXPECT_EQ(dto.diffusion.percentages, coreResult.diffusion.percentages);
    EXPECT_EQ(dto.diffusion.visibility, coreResult.diffusion.visibility);
    EXPECT_EQ(dto.diffusion.energyByTriangleTime, coreResult.diffuseEnergyByTriangleTime);
    EXPECT_EQ(dto.diffusion.timeStepMs, 1);
}

TEST(GuiDiffusion, ResultDtoExponeTriangulosCoreParaHeatmap) {
    core::SimulationResult coreResult;
    coreResult.success = true;
    coreResult.message = "ok";
    coreResult.diffusionTriangles = {
        {10, 7, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}},
        {11, 7, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 1.0, 0.0}}
    };
    coreResult.diffuseEnergyByTriangleTime = {{0.0, 2.0, 4.0}, {0.0, 8.0, 2.0}};
    coreResult.reflectionRays = {
        {{0.25, 0.25, 1.0}, {0.25, 0.25, 0.0}, 10.0, 1}
    };

    GuiScenario scenario = minimalScenario();
    scenario.planes[0].absorption = 0.5f;
    const SimulationResultDto dto = gui::coremappers::toGuiResult(coreResult, scenario);

    ASSERT_EQ(dto.overlay.triangleEnergy.size(), 7u);
    EXPECT_TRUE(dto.overlay.triangleEnergy[0].hasGeometry);
    EXPECT_EQ(dto.overlay.triangleEnergy[0].planeId, 7);
    EXPECT_EQ(dto.overlay.triangleEnergy[0].triangleId, 10);
    EXPECT_NEAR(dto.overlay.triangleEnergy[0].energy, 0.0f, 1e-6f);
    EXPECT_NEAR(dto.overlay.triangleEnergy[0].timeSeconds, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(dto.overlay.triangleEnergy[0].vertices[1].x, 1.0f);

    EXPECT_TRUE(dto.overlay.triangleEnergy[1].hasGeometry);
    EXPECT_EQ(dto.overlay.triangleEnergy[1].triangleId, 11);
    EXPECT_NEAR(dto.overlay.triangleEnergy[1].energy, 0.0f, 1e-6f);

    EXPECT_TRUE(dto.overlay.triangleEnergy[2].hasGeometry);
    EXPECT_EQ(dto.overlay.triangleEnergy[2].triangleId, 10);
    EXPECT_NEAR(dto.overlay.triangleEnergy[2].energy, 0.625f, 1e-6f);
    EXPECT_NEAR(dto.overlay.triangleEnergy[2].timeSeconds, 0.001f, 1e-6f);

    EXPECT_TRUE(dto.overlay.triangleEnergy[5].hasGeometry);
    EXPECT_EQ(dto.overlay.triangleEnergy[5].triangleId, 11);
    EXPECT_NEAR(dto.overlay.triangleEnergy[5].energy, 0.5f, 1e-6f);
    EXPECT_NEAR(dto.overlay.triangleEnergy[5].timeSeconds, 0.001f, 1e-6f);
}

TEST(GuiDiffusion, CoreServicePreservaCoeficienteDifusionDesdeInputGui) {
    auto fake = std::make_unique<CapturingCoreService>();
    CapturingCoreService* fakePtr = fake.get();
    CoreSimulationService service(std::move(fake));

    GuiScenario scenario = minimalScenario();
    scenario.simulationConfig.diffusionCoefficient = 0.73f;

    service.start(scenario);

    EXPECT_NEAR(fakePtr->lastConfig.diffusionCoefficient, 0.73, 1e-6);
}
