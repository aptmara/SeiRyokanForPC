/*****************************************************************//**
 * @file   Window.cpp
 * @brief  Windowクラスの実装
 * 
 * @author 浅野勇生
 * @date   2025/7/16
 *********************************************************************/
#include "Window.h"

bool Window::Create(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
    const wchar_t CLASS_NAME[] = L"MyGameWindowClass";

    // ウィンドウクラスを定義
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    // クラス登録
    RegisterClass(&wc);

    // クライアント領域を1920×1080に調整
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"MyGame",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        this // このWindowクラスのポインタを渡す（WM_NCCREATEで取得）
    );

    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, nCmdShow);
    return true;
}

void Window::ProcessMessage()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            m_shouldQuit = true;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* window = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        // CreateWindowExで渡されたthisポインタを取得
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = static_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
