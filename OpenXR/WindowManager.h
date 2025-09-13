#pragma once

#include <Windows.h>

class WindowManager
{
public:

    WindowManager(int width, int height, const wchar_t* title);
    ~WindowManager();

    bool ProcessMessages() const;

    HWND GetHandle() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;    //ƒnƒ“ƒhƒ‹

    int m_width;
    int m_height;

};


