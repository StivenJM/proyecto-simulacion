#pragma once
#include "ServiceMode.h"

namespace core {

// Configuracion global de la aplicacion.
struct AppConfig {
    ServiceMode serviceMode        = ServiceMode::Mock;
    int         defaultDurationMs  = 1000;
    double      defaultSoundSpeed  = 340.0;
    int         defaultRayCount    = 642;
    bool        debugEnabled       = false;
};

// Retorna la configuracion por defecto de la aplicacion.
AppConfig loadAppConfig();

} // namespace core
