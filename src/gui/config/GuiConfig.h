#pragma once

namespace gui {

enum class GuiServiceProvider {
    Mock,
    Core,
};

struct GuiConfig {
    GuiServiceProvider serviceProvider = GuiServiceProvider::Mock;
};

}  // namespace gui
