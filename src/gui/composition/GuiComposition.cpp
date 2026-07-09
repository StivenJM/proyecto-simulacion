#include "GuiComposition.h"

#include "gui/factories/PlaneServiceFactory.h"
#include "gui/factories/ReceiverServiceFactory.h"
#include "gui/factories/ScenarioServiceFactory.h"
#include "gui/factories/SimulationServiceFactory.h"
#include "gui/factories/SourceServiceFactory.h"

namespace gui {

GuiServices GuiComposition::createServices(const GuiConfig& config)
{
    GuiServices services;
    services.planeService = PlaneServiceFactory::create(config);
    services.sourceService = SourceServiceFactory::create(config);
    services.receiverService = ReceiverServiceFactory::create(config);
    services.scenarioService = ScenarioServiceFactory::create(config);
    services.simulationService = SimulationServiceFactory::create(config);
    return services;
}

}  // namespace gui
