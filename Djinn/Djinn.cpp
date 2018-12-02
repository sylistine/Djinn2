#include <iostream>
#include <cstdio>

#include "Djinn.h"

#include "WindowsContainer.h"
#include "Graphics.h"

#include "Utilities.h"


using namespace Djinn;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int cmdshow)
{
    Logger::Write(L"Djinn Renderer v0.2");

    try {
        WindowsContainer container(hInstance);
        container.Initialize();

        Graphics::Context().Initialize(
            container.GetWindowHandler(),
            container.GetWindowWidth(), container.GetWindowHeight());

        while (!container.IsApplicationQuitting()) {
            container.Update();
            // Handle handle resize messages.
            Graphics::Context().Update();
        }
    } catch (std::exception &e) {
        Logger::Write(Utilities::ANSI2UNICODE(e.what()).c_str());
    }

    return 0;
}
