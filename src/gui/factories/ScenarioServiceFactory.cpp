#include "ScenarioServiceFactory.h"

#include "gui/services/implementations/core/CoreScenarioService.h"
#include "gui/services/implementations/mock/MockScenarioService.h"

namespace gui {

std::unique_ptr<IScenarioService> ScenarioServiceFactory::create(const GuiConfig& config)
{
    if (config.serviceProvider == GuiServiceProvider::Core) {
        return std::make_unique<CoreScenarioService>();
    }

    return std::make_unique<MockScenarioService>();
}

}  // namespace gui
