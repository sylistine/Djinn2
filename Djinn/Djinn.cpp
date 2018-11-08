#include <iostream>
#include <cstdio>

#include "Djinn.h"

#include "WindowsContainer.h"
#include "Graphics.h"

#include "Utilities.h"


using namespace Djinn;

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int cmdshow)
{
    Logger::Log(L"Djinn Renderer v0.2");

    try {
        WindowsContainer container(hInstance);
        container.Initialize();

        Graphics graphics(container.GetWindowHandler(), container.GetWindowWidth(), container.GetWindowHeight());
        graphics.Initialize();

        while (!container.IsApplicationQuitting()) {
            container.Update();
            // Handle handle resize messages.
            graphics.Update();
        }
    } catch (std::exception &e) {
        Logger::Log(Utilities::ANSI2UNICODE(e.what()).c_str());
    }

    return 0;
}
#else
int main()
{
    Logger::Log("Non-windows platforms not yet supported.");
    return 0;
}
#endif
