#pragma once
#include "ServiceMode.h"

namespace config {

struct AppConfig {
    ServiceMode serviceMode   = ServiceMode::Mock;
    int         defaultDurationMs  = 1000;
    double      defaultSoundSpeed  = 340.0;
    int         defaultRayCount    = 642;
    bool        debugEnabled       = false;
};

AppConfig loadAppConfig();

} // namespace config
