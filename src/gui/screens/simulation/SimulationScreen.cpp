#include "SimulationScreen.h"

namespace gui {

bool SimulationScreen::start()
{
    if (started_) {
        return false;
    }

    started_ = true;
    return true;
}

void SimulationScreen::reset()
{
    started_ = false;
}

bool SimulationScreen::isStarted() const
{
    return started_;
}

}  // namespace gui
