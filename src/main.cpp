#include <iostream>

#include "gui/App.h"

namespace {

int runApplication()
{
    gui::App app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize the acoustic simulator GUI.\n";
        return 1;
    }

    app.run();
    return 0;
}

}  // namespace

int main()
{
    return runApplication();
}

#if defined(_WIN32)
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return runApplication();
}
#endif
