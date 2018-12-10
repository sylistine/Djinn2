#include <iostream>
#include <cstdio>

#include "Djinn.h"

#include "WindowsContainer.h"
#include "Gfx.h"

#include "Utilities.h"


using namespace Djinn;
using namespace Djinn::Graphics;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int cmdshow)
{
    Logger::Write(L"Djinn Renderer v0.2");

    try {
        WindowsContainer container(hInstance);
        container.Initialize();

        Gfx::Context().Initialize(
            container.GetWindowHandler(),
            container.GetWindowWidth(), container.GetWindowHeight());

        while (!container.IsApplicationQuitting()) {
            container.Update();
            // Handle handle resize messages.
            Gfx::Context().Update();
        }
    } catch (std::exception &e) {
        Logger::Write(Utilities::ANSI2UNICODE(e.what()).c_str());
    }

    return 0;
}
