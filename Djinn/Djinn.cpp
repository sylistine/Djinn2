#include <iostream>
#include <cstdio>

#include "Djinn.h"

#include "WindowsContainer.h"
#include "Graphics.h"


using namespace Djinn;

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int cmdshow)
{
    try {
        WindowsContainer container(hInstance);
        container.Initialize();

        Graphics graphics;
        graphics.Initialize();

        while (!container.IsApplicationQuitting()) {
            container.Update();
            // Handle handle resize messages.
            graphics.Update();
        }
    } catch (std::exception e) {
        Logger::Log(e.what());
    }

    return 0;
}
#else
int main()
{
    return 0;
}
#endif
