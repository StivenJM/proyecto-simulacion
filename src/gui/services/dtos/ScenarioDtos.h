#pragma once

#include "PlaneDtos.h"

#include <vector>

namespace gui {

struct ScenarioDto {
    std::vector<PlaneDto> planes;
};

}  // namespace gui
