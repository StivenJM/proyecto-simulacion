#include "ReceiverServiceFactory.h"

#include "gui/services/implementations/core/CoreReceiverService.h"
#include "gui/services/implementations/mock/MockReceiverService.h"

namespace gui {

std::unique_ptr<IReceiverService> ReceiverServiceFactory::create(const GuiConfig& config)
{
    if (config.serviceProvider == GuiServiceProvider::Core) {
        return std::make_unique<CoreReceiverService>();
    }

    return std::make_unique<MockReceiverService>();
}

}  // namespace gui
