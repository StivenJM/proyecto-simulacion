#pragma once

namespace core {

// Modo de servicio: Mock usa datos falsos, Core usa calculos reales.
enum class ServiceMode {
    Mock,
    Core
};

} // namespace core
