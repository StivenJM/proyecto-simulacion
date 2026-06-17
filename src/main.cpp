#include <iostream>

#include "gui/App.h"

int main()
{
    gui::App app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize the acoustic simulator GUI.\n";
        return 1;
    }

    app.run();
    return 0;
}
