#pragma once

#include "gui/config/GuiConfig.h"
#include "gui/services/IScenarioService.h"

#include <memory>

namespace gui {

class ScenarioServiceFactory {
public:
    static std::unique_ptr<IScenarioService> create(const GuiConfig& config);
};

}  // namespace gui
