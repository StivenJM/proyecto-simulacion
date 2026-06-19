#include "GuiComposition.h"

#include "gui/factories/PlaneServiceFactory.h"
#include "gui/factories/ScenarioServiceFactory.h"

namespace gui {

GuiServices GuiComposition::createServices(const GuiConfig& config)
{
    GuiServices services;
    services.planeService = PlaneServiceFactory::create(config);
    services.scenarioService = ScenarioServiceFactory::create(config);
    return services;
}

}  // namespace gui
