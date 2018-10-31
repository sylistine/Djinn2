#pragma once

#include <cassert>

#include <Windows.h>
#include <WindowsX.h>

#include "Timer.h"

namespace Djinn
{
    class WindowsContainer
    {
    public:
        // tors
        WindowsContainer(HINSTANCE hInstance);
        ~WindowsContainer();
        // statics
        static WindowsContainer *windowsApp;
        static WindowsContainer *GetApp();
        //publics
        bool Initialize();
        int Update();
        LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
        bool IsApplicationQuitting() const { return isApplicationQuitting; };
    private:
        // Windows parameters.
        HINSTANCE hInstance;
        HWND hWnd;
        MSG msg;
        int windowWidth = 800;
        int windowHeight = 600;
        // Initialization.
        bool initialized = false;
        bool InitializeWindow();
        // WndProc message handlers.
        void OnMouseDown(WPARAM, int, int);
        void OnMouseUp(WPARAM, int, int);
        void OnMouseMove(WPARAM, int, int);
        // game state
        bool paused = false;
        bool minimized = false;
        bool maximized = false;
        bool resizing = false;
        bool isApplicationQuitting = false;
    };
}
