#pragma once

#include "gui/services/dtos/SimulationDtos.h"

namespace gui {

class ISimulationService {
public:
    virtual ~ISimulationService() = default;

    virtual SimulationStateDto start() = 0;
};

}  // namespace gui
