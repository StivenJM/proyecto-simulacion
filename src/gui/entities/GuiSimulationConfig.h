#pragma once

namespace gui {

struct GuiSimulationConfig {
    int rayCount = 128;
    int meshSubdivisions = 1;
    float globalAbsorption = 0.2f;
    float diffusionCoefficient = 0.1f;
};

}  // namespace gui
