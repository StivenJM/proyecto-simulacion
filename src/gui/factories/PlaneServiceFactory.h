#pragma once

#include "gui/config/GuiConfig.h"
#include "gui/services/IPlaneService.h"

#include <memory>

namespace gui {

class PlaneServiceFactory {
public:
    static std::unique_ptr<IPlaneService> create(const GuiConfig& config);
};

}  // namespace gui
