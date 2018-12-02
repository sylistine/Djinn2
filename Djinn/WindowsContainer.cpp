#include "WindowsContainer.h"

using namespace Djinn;

LRESULT WINAPI MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return WindowsContainer::GetApp()->WndProc(hWnd, msg, wParam, lParam);
}


WindowsContainer::WindowsContainer(HINSTANCE hInstance) : hInstance(hInstance)
{
    assert(windowsApp == nullptr);
    windowsApp = this;
}


WindowsContainer::~WindowsContainer() = default;


WindowsContainer *WindowsContainer::windowsApp = nullptr;


WindowsContainer *WindowsContainer::GetApp()
{
    return windowsApp;
}


bool WindowsContainer::Initialize()
{
    Logger::Write(L"Platform: Win32");

    

    if (!initialized) {
        if (!InitializeWindow()) {
            return false;
        }
    }

    msg = { 0 };
    Timer::GetTimer()->Reset();

    return true;
}


bool WindowsContainer::InitializeWindow()
{
    auto wndClassName = L"DjinnRendererWindow";
    auto wndTitle = L"Djinn Renderer";

    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
    wc.lpszMenuName = 0;
    wc.lpszClassName = wndClassName;

    if (!RegisterClass(&wc)) {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT rect = { 0, 0, windowWidth, windowHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    hWnd = CreateWindow(
        wndClassName,
        wndTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width, height,
        0, 0,
        hInstance,
        0);
    if (!hWnd) {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    return true;
}


int WindowsContainer::Update()
{
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } else {
        Timer::GetTimer()->Tick();
    }

    return static_cast<int>(msg.wParam);
}


void WindowsContainer::OnMouseDown(WPARAM wParam, int x, int y)
{

}


void WindowsContainer::OnMouseUp(WPARAM wParam, int x, int y)
{

}


void WindowsContainer::OnMouseMove(WPARAM wParam, int x, int y)
{

}


LRESULT WindowsContainer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    /* Handling the following messages:
    * ACTIVATE
    * SIZE
    * ENTERSIZEMOVE
    * EXITSIZEMOVE
    * DESTROY
    * MENUCHAR
    * GETMINMAXINFO
    * L- M- RBUTTONDOWN
    * L- M- RBUTTONUP
    * MOUSEMOVE
    * KEYUP
    */

    switch (msg) {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) {
            Timer::GetTimer()->Stop();
            paused = true;
        } else {
            Timer::GetTimer()->Start();
            paused = false;
        }
        return 0;
    case WM_SIZE:
        //windowWidth = LOWORD(lParam);
        //windowHeight = HIWORD(lParam);
        if (wParam == SIZE_MINIMIZED) {
            paused = true;
            minimized = true;
            maximized = false;
        } else if (wParam == SIZE_MAXIMIZED) {
            paused = false;
            minimized = false;
            maximized = true;
            // TODO: make sure graphpics updates the client dimensions.
        } else if (wParam == SIZE_RESTORED) {
            if (resizing) {} else {
                if (minimized) {
                    paused = false;
                    minimized = false;
                } else if (maximized) {
                    paused = false;
                    maximized = false;
                }
                // TODO: make sure graphpics updates the client dimensions.
            }
        }
        return 0;
    case WM_ENTERSIZEMOVE:
        paused = true;
        resizing = true;
        Timer::GetTimer()->Stop();
        return 0;
    case WM_EXITSIZEMOVE:
        paused = false;
        resizing = false;
        Timer::GetTimer()->Start();
        // TODO: Make sure graphics handles this?
        //if (gfxRHI != nullptr) {
        //    gfxRHI->SetClientDimensions(windowWidth, windowHeight);
        //}
        return 0;
    case WM_DESTROY:
        isApplicationQuitting = true;
        PostQuitMessage(0);
        return 0;
    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_KEYUP:
        // TODO: Handle input. Make it event based, maybe?
        if (wParam == VK_ESCAPE) {
            isApplicationQuitting = true;
            PostQuitMessage(0);
        }
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
