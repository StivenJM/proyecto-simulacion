#pragma once

#include "gui/config/GuiConfig.h"
#include "gui/services/ISourceService.h"

#include <memory>

namespace gui {

class SourceServiceFactory {
public:
    static std::unique_ptr<ISourceService> create(const GuiConfig& config);
};

}  // namespace gui
