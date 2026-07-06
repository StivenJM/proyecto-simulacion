#include "MockSimulationService.h"

namespace core {

SimulationResult MockSimulationService::runSimulation(
    const ScenarioData&     scenario,
    const SimulationConfig& config
) {
    SimulationResult result;
    result.success = true;
    result.message = "Mock simulation completed.";

    result.reflectionRays = {
        {{0.0, 0.5, 0.0}, {2.0, 0.5, 0.0}, 1.0,  0},
        {{2.0, 0.5, 0.0}, {2.0, 2.5, 0.0}, 0.8,  6},
        {{2.0, 2.5, 0.0}, {0.0, 2.5, 0.0}, 0.6, 12},
        {{0.0, 2.5, 0.0}, {0.0, 0.5, 1.5}, 0.4, 18},
        {{0.0, 0.5, 1.5}, {1.5, 0.5, 3.0}, 0.2, 24},
    };

    int receiverId = scenario.receivers.empty() ? 0 : scenario.receivers[0].id;
    result.receiverEnergy = {
        {receiverId,  50, 0.40},
        {receiverId, 150, 0.25},
        {receiverId, 300, 0.12},
        {receiverId, 500, 0.05},
        {receiverId, 800, 0.02},
    };
    result.totalReceiverEnergy = 0.84;

    result.triangleEnergy = {
        {0,   0, 0.9},
        {1,  50, 0.7},
        {2, 100, 0.5},
        {3, 150, 0.3},
        {4, 200, 0.1},
    };

    result.diffusion.distances   = {{0.0, 3.0}, {3.0, 0.0}};
    result.diffusion.timesMs     = {{0, 9}, {9, 0}};
    result.diffusion.percentages = {{0.0, 1.0}, {1.0, 0.0}};
    result.diffusion.visibility  = {{false, true}, {true, false}};

    result.lostEnergy = 0.16;
    return result;
}

} // namespace core
