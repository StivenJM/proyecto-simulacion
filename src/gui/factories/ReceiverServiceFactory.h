#pragma once

#include "gui/config/GuiConfig.h"
#include "gui/services/IReceiverService.h"

#include <memory>

namespace gui {

class ReceiverServiceFactory {
public:
    static std::unique_ptr<IReceiverService> create(const GuiConfig& config);
};

}  // namespace gui
