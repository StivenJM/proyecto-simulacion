#include "PlaneServiceFactory.h"

#include "gui/services/implementations/core/CorePlaneService.h"
#include "gui/services/implementations/mock/MockPlaneService.h"

namespace gui {

std::unique_ptr<IPlaneService> PlaneServiceFactory::create(const GuiConfig& config)
{
    if (config.serviceProvider == GuiServiceProvider::Core) {
        return std::make_unique<CorePlaneService>();
    }

    return std::make_unique<MockPlaneService>();
}

}  // namespace gui
