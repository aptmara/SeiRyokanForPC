/*****************************************************************//**
 * @file   Window.h
 * @brief  WindowçÏê¨ÉNÉâÉXÇÃíËã`
 * 
 * @author êÛñÏóEê∂
 * @date   2025/7/16
 *********************************************************************/
#pragma once
#include <windows.h>

class Window
{
public:
    bool Create(HINSTANCE hInstance, int nCmdShow, int width = 1920, int height = 1080);
    void ProcessMessage();
    HWND GetHwnd() const { return m_hwnd; }

    bool ShouldQuit() const { return m_shouldQuit; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    HWND m_hwnd = nullptr;
    bool m_shouldQuit = false;
};
