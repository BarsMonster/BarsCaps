﻿// Copyright 2025 Mikhail Svarichevsky 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED “AS IS”,
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// CapsLockLanguageSwitcher.cpp
//
// This is a single‐file Unicode Win32 application that installs a low‐level
// keyboard hook. When the user presses the CapsLock key without Alt,
// the app prevents the usual CapsLock toggle and instead switches
// sequentially between the installed keyboard layouts. (If only 2 layouts
// are installed, it toggles between them.) If the user holds Alt while
// pressing CapsLock, then the normal CapsLock toggling behavior is allowed.
// The tray icon’s context menu now contains an "About" option that shows a
// message box with multi‑line text and an "Exit" option to close the app.

#define APP_VERSION   "1.01"

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")

// IDs and custom messages
#define TRAY_ICON_UID 1001
#define ID_EXIT       4001
#define ID_ABOUT      4002
#define ID_GITHUB     4003
#define ID_PI314      4004
#define TRAY_ICON_MSG (WM_APP + 1)

#if defined(_M_IX86)
    #define PLATFORM_NAME "x86"
#elif defined(_M_X64)
    #define PLATFORM_NAME "x64"
#elif defined(_M_ARM64)
    #define PLATFORM_NAME "ARM64"
#else
    #define PLATFORM_NAME "Unknown"
#endif

// Global variables
HHOOK          g_hHook = NULL;
HWND           g_hWnd  = NULL;
NOTIFYICONDATA g_nid  = { 0 };

// Global flag to track if a CapsLock press began with Alt.
static bool g_bAltCapsCombination = false;

void InitTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);

//
// SwitchLanguage: retrieves the current keyboard layout, gets the list
// of installed layouts, determines the “next” one, and posts a message to
// the foreground window to change the layout.
//
void SwitchLanguage()
{
    // Get the current layout from the foreground window's thread.
    HWND hwndForeground = GetForegroundWindow();
    DWORD threadId = 0;
    if (hwndForeground)
        threadId = GetWindowThreadProcessId(hwndForeground, NULL);
    HKL currentHKL = GetKeyboardLayout(threadId);

    // Retrieve installed layouts.
    int count = GetKeyboardLayoutList(0, NULL);
    if (count <= 0)
        return;
    if (count > 16)
        count = 16;

    HKL layouts[16];
    GetKeyboardLayoutList(count, layouts);

    // Find the current layout in the list.
    int curIndex = -1;
    for (int i = 0; i < count; i++) {
        if (layouts[i] == currentHKL) {
            curIndex = i;
            break;
        }
    }
    if (curIndex < 0)
        curIndex = 0; // fallback

    // Compute the index of the next layout (wrap-around).
    int nextIndex = (curIndex + 1) % count;
    HKL nextHKL = layouts[nextIndex];

    // Post a WM_INPUTLANGCHANGEREQUEST to the foreground window.
    if (hwndForeground)
        PostMessage(hwndForeground, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)nextHKL);
    else
        ActivateKeyboardLayout(nextHKL, 0);
}

//
// LowLevelKeyboardProc: intercepts keyboard events. For the CapsLock key,
// if it was pressed without Alt (determined on key‑down), we swallow both the
// key‑down and key‑up events and switch the input language. Otherwise, if
// Alt was held when CapsLock was pressed, we let all events pass through.
//
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKb = (KBDLLHOOKSTRUCT*)lParam;
        if (pKb->vkCode == VK_CAPITAL)
        {
            // For key down events:
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            {
                // Check if Alt is held down using GetAsyncKeyState.
                if (GetAsyncKeyState(VK_MENU) < 0)
                {
                    // Mark that this CapsLock event is an Alt+CapsLock combination.
                    g_bAltCapsCombination = true;
                    // Let Windows handle it normally.
                    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
                }
                else
                {
                    // No Alt: switch language.
                    SwitchLanguage();
                    // Swallow the event so the CapsLock state isn’t toggled.
                    return 1;
                }
            }
            // For key up events:
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
            {
                if (g_bAltCapsCombination)
                {
                    // This key up is part of an Alt+CapsLock event; let it pass.
                    g_bAltCapsCombination = false;
                    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
                }
                else
                {
                    // Swallow the key up if it wasn’t an Alt combination.
                    return 1;
                }
            }
        }
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// Function to open a URL in the default browser
void OpenURL(LPCWSTR url)
{
    ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
}

int GetNotificationIconSize() {
    // Get the monitor hosting the taskbar (usually primary monitor)
    return GetSystemMetrics(SM_CXSMICON); // Typically 16, but is different for DPI-aware apps (which we are)
}

// Create an icon (16x16+) with text on a transparent background
HICON CreateLetterAIcon(LPCWSTR  logo)
{
    const int icon_size = GetNotificationIconSize();
    const int temp_size = icon_size * 4; // Larger temporary bitmap for accurate text rendering
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcTemp = CreateCompatibleDC(hdcScreen);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = temp_size;
    bmi.bmiHeader.biHeight = -temp_size;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    VOID* pBits = NULL;
    HBITMAP hTempBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    SelectObject(hdcTemp, hTempBitmap);
    
    DWORD* pixels = (DWORD*)pBits;
    
    SetBkMode(hdcTemp, TRANSPARENT);
    SetTextColor(hdcTemp, RGB(0, 0, 0));
    
    HFONT hFont = NULL, hOldFont = NULL;
    int targetHeight = icon_size - 2;
    int bestFontSize = 10;
    int y_offset = 0, x_offset = 0;
    
    for (int fontSize = 10; fontSize <= icon_size * 10; fontSize++) {
        if (hFont) DeleteObject(hFont);
        hFont = CreateFontW(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial");
        SelectObject(hdcTemp, hFont);
        
        // Clear bitmap
        memset(pixels, 0xFF, temp_size * temp_size * sizeof(DWORD));
        
        RECT rect = {5, 5, temp_size, temp_size};
        DrawTextW(hdcTemp, logo, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP);//L"+"Ӂ
        
        int minY = temp_size, maxY = 0;
        int minX = temp_size, maxX = 0;
        for (int y = 0; y < temp_size; y++) {
            for (int x = 0; x < temp_size; x++) {
                int index = y * temp_size + x;
                BYTE alpha = (BYTE)(pixels[index] & 0xFF);
                if (alpha < 128) {
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                }
            }
        }
        int actualHeight = maxY - minY + 1;
        
        if (actualHeight > targetHeight) {
            bestFontSize = fontSize - 1;
            break;
        } else
        {
            y_offset=5-minY + (icon_size - (maxY-minY+1))/2;
            x_offset=5-minX + (icon_size - (maxX-minX+1))/2;
        }
    }
    
    DeleteObject(hFont);
    hFont = CreateFontW(bestFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial");
    
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    bmi.bmiHeader.biWidth = icon_size;
    bmi.bmiHeader.biHeight = -icon_size;
    VOID* pBitsFinal = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBitsFinal, NULL, 0);
    SelectObject(hdcMem, hBitmap);
    SelectObject(hdcMem, hFont);
    memset(pBitsFinal, 0xFF, icon_size * icon_size * sizeof(DWORD)); 
    
    RECT rect = {x_offset, y_offset, icon_size+10, icon_size+10};
    DrawTextW(hdcMem, logo, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_TOP);//Ӂ
    
    DWORD* pixelsFinal = (DWORD*)pBitsFinal;
    for (int y = 0; y < icon_size; y++) {
        for (int x = 0; x < icon_size; x++) {
            int index = y * icon_size + x;
            BYTE alpha = ~(BYTE)(pixelsFinal[index] & 0xFF);
//            if(alpha < 20) alpha = 20;
//            if(alpha > 240) alpha = 240;
            pixelsFinal[index] = (alpha << 24) | 0xFFFFFF;
        }
    }
    
    DeleteObject(hFont);
    DeleteDC(hdcTemp);
    DeleteObject(hTempBitmap);
    
    HBITMAP hMask = CreateBitmap(icon_size, icon_size, 1, 1, NULL);
    HDC hdcMask = CreateCompatibleDC(hdcScreen);
    SelectObject(hdcMask, hMask);
    SetBkColor(hdcMask, RGB(0, 0, 0));
    BitBlt(hdcMask, 0, 0, icon_size, icon_size, hdcMem, 0, 0, SRCCOPY);
    
    ICONINFO iconInfo = {0};
    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = hMask;
    iconInfo.hbmColor = hBitmap;
    
    HICON hIcon = CreateIconIndirect(&iconInfo);
    
    DeleteDC(hdcMask);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hBitmap);
    DeleteObject(hMask);
    
    return hIcon;
}

// Initialize the tray icon.
void InitTrayIcon(HWND hWnd)
{
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_nid.uCallbackMessage = TRAY_ICON_MSG;
    lstrcpy(g_nid.szTip, TEXT("Language Switcher"));
    g_nid.hIcon = CreateLetterAIcon(L"Ӂ");
    
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

// Remove the tray icon.
void RemoveTrayIcon(HWND hWnd)
{
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    if (g_nid.hIcon)
    {
        DestroyIcon(g_nid.hIcon);
        g_nid.hIcon = NULL;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case TRAY_ICON_MSG:
        if (lParam == WM_RBUTTONUP)
        {
            // Show a context menu with options
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                InsertMenuW(hMenu, -1, MF_BYPOSITION, ID_ABOUT, L"About");
                InsertMenuW(hMenu, -1, MF_BYPOSITION, ID_GITHUB, L"GitHub - BarsCaps");
                InsertMenuW(hMenu, -1, MF_BYPOSITION, ID_PI314, L"Homepage - 3.14.by");
                InsertMenuW(hMenu, -1, MF_BYPOSITION, ID_EXIT,  L"Exit");

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_ABOUT:
            MessageBoxW(hwnd,
                L"v" APP_VERSION L" " PLATFORM_NAME L"   " __DATE__ L"\n"
                L"Switches keyboard languages when CapsLock is pressed.\n"
                L"Use Alt+CapsLock to toggle CapsLock instead.\n\n",
                L"About BarsCaps Language Switcher", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_GITHUB:
            OpenURL(L"https://github.com/BarsMonster/BarsCaps");
            break;
        case ID_PI314:
            OpenURL(L"https://3.14.by");
            break;
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_DESTROY:
        RemoveTrayIcon(hwnd);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

//
// wWinMain: entry point. We register a hidden window (using a message-only
// window), add our tray icon, install our low-level keyboard hook, run the
// message loop, and then clean up.
//
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    SetProcessDPIAware();
    // Register a window class for our hidden window.
    const wchar_t CLASS_NAME[] = L"BarsCapsLanguageSwitcherWindowClass";
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc   = WindowProc;
    wcex.hInstance     = hInstance;
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);

    // Create a message-only window (invisible, no taskbar icon).
    g_hWnd = CreateWindowExW(0, CLASS_NAME, L"BarsCaps CapsLock Keyboard Layout Switcher", 0,
                             0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (!g_hWnd)
        return 1;

    // Initialize the tray icon.
    InitTrayIcon(g_hWnd);

    // Install the low-level keyboard hook.
    g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    if (!g_hHook)
    {
        RemoveTrayIcon(g_hWnd);
        return 1;
    }

    // Enter the message loop.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup: remove the hook.
    UnhookWindowsHookEx(g_hHook);
    return (int)msg.wParam;
}
