#pragma once

namespace gui {

class SimulationScreen {
public:
    bool start();
    void reset();
    bool isStarted() const;

private:
    bool started_ = false;
};

}  // namespace gui
