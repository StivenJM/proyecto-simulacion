#include "SourceServiceFactory.h"

#include "gui/services/implementations/core/CoreSourceService.h"
#include "gui/services/implementations/mock/MockSourceService.h"

namespace gui {

std::unique_ptr<ISourceService> SourceServiceFactory::create(const GuiConfig& config)
{
    if (config.serviceProvider == GuiServiceProvider::Core) {
        return std::make_unique<CoreSourceService>();
    }

    return std::make_unique<MockSourceService>();
}

}  // namespace gui
