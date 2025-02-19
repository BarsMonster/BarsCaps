// Copyright 2025 Mikhail Svarichevsky 
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

#define APP_VERSION   "1.0"

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <shellapi.h>

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

// Create an icon (16x16) with a drawn letter 'A'
HICON CreateLetterAIcon()
{
    const int width = 16, height = 16;
    
    // Create a device context and a bitmap.
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);
    
    // Fill background (white background in this example)
    HBRUSH hBrush = CreateSolidBrush(RGB(255,255,255));
    RECT rect = {0, 0, width, height};
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);
    
    // Draw the letter 'A'
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, RGB(0,0,0));
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD,
                             FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH, TEXT("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
    
    DrawText(hdcMem, TEXT("A"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    
    SelectObject(hdcMem, hOldFont);
    DeleteObject(hFont);
    
    // Create an icon from the bitmap.
    ICONINFO iconInfo = {0};
    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = hBitmap;
    iconInfo.hbmColor = hBitmap;
    
    HICON hIcon = CreateIconIndirect(&iconInfo);
    
    // Cleanup
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hBitmap);
    
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
    g_nid.hIcon = CreateLetterAIcon();
    
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
            OpenURL(L"https://github.com/barscaps");
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
    // Register a window class for our hidden window.
    const wchar_t CLASS_NAME[] = L"CapsLockLanguageSwitcherWindowClass";
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc   = WindowProc;
    wcex.hInstance     = hInstance;
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);

    // Create a message-only window (invisible, no taskbar icon).
    g_hWnd = CreateWindowExW(0, CLASS_NAME, L"CapsLock Language Switcher", 0,
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
