#include "CoreSimulationService.h"

#include "core/AppConfig.h"
#include "core/ServiceFactory.h"
#include "core/ServiceMode.h"
#include "gui/services/implementations/core/mappers/ResultMapper.h"
#include "gui/services/implementations/core/mappers/ScenarioMapper.h"

#include <utility>

namespace gui {
namespace {

double clamp01(double value)
{
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}

}  // namespace

CoreSimulationService::CoreSimulationService()
    : CoreSimulationService([] {
          core::AppConfig config;
          config.serviceMode = core::ServiceMode::Core;
          return core::ServiceFactory::createSimulationService(config);
      }())
{
}

CoreSimulationService::CoreSimulationService(std::unique_ptr<core::ISimulationService> coreService)
    : coreService_(std::move(coreService))
{
}

SimulationResultDto CoreSimulationService::start(const GuiScenario& scenario)
{
    core::SimulationConfig config;
    config.durationMs = 1000;
    config.soundSpeed = 340.0;
    config.rayCount = scenario.simulationConfig.rayCount < 1 ? 1 : scenario.simulationConfig.rayCount;
    config.meshSubdivisions = scenario.simulationConfig.meshSubdivisions < 1 ? 1 : scenario.simulationConfig.meshSubdivisions;
    config.diffusionCoefficient = clamp01(static_cast<double>(scenario.simulationConfig.diffusionCoefficient));

    const core::ScenarioData coreScenario = coremappers::toCoreScenario(scenario);
    SimulationResultDto result = coremappers::toGuiResult(coreService_->runSimulation(coreScenario, config), scenario);
    result.configuredRayCount = config.rayCount;
    return result;
}

}  // namespace gui
