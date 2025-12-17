/**
 * Chinese comments may cause compilation errors in some cases, so this project uses English comments. Chinese users are responsible for translating them themselves.
 * WindowHider - Window Hiding DLL
 *
 * Exports:
 *   - SetWindowVisibility(HWND hwnd, BOOL hide)
 *   - HideAllWindows()
 *   - ShowAllWindows()
 *   - HideFromTaskbar(HWND hwnd, BOOL hide)
 *
 * Requirements: Windows 10 v2004+
 */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/**
 * Check if window is a valid app window (visible, top-level, has title)
 */
static BOOL IsValidAppWindow(HWND hwnd) {
    if (!IsWindowVisible(hwnd)) {
        return FALSE;
    }

    HWND parent = GetParent(hwnd);
    if (parent != NULL && parent != GetDesktopWindow()) {
        return FALSE;
    }

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (style & WS_CHILD) {
        return FALSE;
    }

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return FALSE;
    }

    WCHAR title[256];
    int len = GetWindowTextW(hwnd, title, 256);
    if (len == 0) {
        return FALSE;
    }

    return TRUE;
}

/**
 * Internal: Set visibility for all windows in current process
 */
static void SetAllWindowsVisibilityInternal(BOOL hide) {
    DWORD currentPID = GetCurrentProcessId();
    DWORD dwAffinity = hide ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;

    HWND hwnd = NULL;
    while ((hwnd = FindWindowEx(NULL, hwnd, NULL, NULL)) != NULL) {
        DWORD windowPID = 0;
        GetWindowThreadProcessId(hwnd, &windowPID);
        if (windowPID == currentPID && IsValidAppWindow(hwnd)) {
            SetWindowDisplayAffinity(hwnd, dwAffinity);
        }
    }
}

/**
 * Set window display affinity (hide from screen capture)
 */
extern "C" __declspec(dllexport) BOOL __stdcall SetWindowVisibility(HWND hwnd, BOOL hide) {
    if (hwnd == NULL) {
        return FALSE;
    }

    DWORD dwAffinity = hide ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
    return SetWindowDisplayAffinity(hwnd, dwAffinity);
}

/**
 * Hide all windows in current process from screen capture
 */
extern "C" __declspec(dllexport) void __stdcall HideAllWindows() {
    SetAllWindowsVisibilityInternal(TRUE);
}

/**
 * Show all windows in current process (restore normal display)
 */
extern "C" __declspec(dllexport) void __stdcall ShowAllWindows() {
    SetAllWindowsVisibilityInternal(FALSE);
}

/**
 * Hide/show window from taskbar
 */
extern "C" __declspec(dllexport) BOOL __stdcall HideFromTaskbar(HWND hwnd, BOOL hide) {
    if (hwnd == NULL) {
        return FALSE;
    }

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (style == 0) {
        return FALSE;
    }

    if (hide) {
        style |= WS_EX_TOOLWINDOW;
        style &= ~WS_EX_APPWINDOW;
    } else {
        style |= WS_EX_APPWINDOW;
        style &= ~WS_EX_TOOLWINDOW;
    }

    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
    return TRUE;
}

/**
 * DLL entry point
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
