/**
 * WindowHider - Window Hiding DLL
 *
 * Hide windows from screen capture/sharing while keeping them visible to the user.
 * Uses SetWindowDisplayAffinity with WDA_EXCLUDEFROMCAPTURE flag.
 *
 * Exports:
 *   - SetWindowVisibility(HWND hwnd, BOOL hide) - Hide/show a specific window
 *   - HideAllWindows() - Hide all windows of current process
 *   - ShowAllWindows() - Show all windows of current process
 *   - HideFromTaskbar(HWND hwnd, BOOL hide) - Hide/show window from taskbar
 *
 * Requirements: Windows 10 v2004+ for proper hiding (older versions show black box)
 */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/**
 * Context structure for EnumWindows callback
 */
typedef struct {
    DWORD targetPID;
    DWORD affinity;
} EnumWindowsContext;

/**
 * Check if window is a valid application window that should be processed.
 * Filters out child windows, tool windows, and windows without titles.
 *
 * @param hwnd Window handle to check
 * @return TRUE if window should be processed, FALSE otherwise
 */
static BOOL IsValidAppWindow(HWND hwnd) {
    // Check if window handle is valid
    if (!IsWindow(hwnd)) {
        return FALSE;
    }

    // Must be visible
    if (!IsWindowVisible(hwnd)) {
        return FALSE;
    }

    // Must be a top-level window (no parent, or parent is desktop)
    HWND parent = GetParent(hwnd);
    if (parent != NULL && parent != GetDesktopWindow()) {
        return FALSE;
    }

    // Must not be a child window
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (style & WS_CHILD) {
        return FALSE;
    }

    // Must not be a tool window (floating toolbars, etc.)
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return FALSE;
    }

    // Must have a title (filters out internal/helper windows)
    WCHAR title[256];
    int len = GetWindowTextW(hwnd, title, 256);
    if (len == 0) {
        return FALSE;
    }

    return TRUE;
}

/**
 * EnumWindows callback function.
 * Sets display affinity for windows belonging to the target process.
 *
 * @param hwnd Current window handle
 * @param lParam Pointer to EnumWindowsContext
 * @return TRUE to continue enumeration, FALSE to stop
 */
static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    EnumWindowsContext* ctx = (EnumWindowsContext*)lParam;

    // Get the process ID of this window
    DWORD windowPID = 0;
    GetWindowThreadProcessId(hwnd, &windowPID);

    // Only process windows belonging to our target process
    if (windowPID == ctx->targetPID) {
        // Check if this is a valid app window we should process
        if (IsValidAppWindow(hwnd)) {
            // Set the display affinity
            SetWindowDisplayAffinity(hwnd, ctx->affinity);
        }
    }

    // Continue enumeration
    return TRUE;
}

/**
 * Internal: Set visibility for all windows in current process.
 * Uses EnumWindows for safe and reliable window enumeration.
 *
 * @param hide TRUE to hide from capture, FALSE to show normally
 */
static void SetAllWindowsVisibilityInternal(BOOL hide) {
    EnumWindowsContext ctx;
    ctx.targetPID = GetCurrentProcessId();
    ctx.affinity = hide ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;

    // Enumerate all top-level windows and apply affinity
    EnumWindows(EnumWindowsCallback, (LPARAM)&ctx);
}

/**
 * Set window display affinity to hide from screen capture.
 * The window remains visible to the user but is excluded from screenshots and screen sharing.
 *
 * @param hwnd Window handle to modify
 * @param hide TRUE to hide from capture, FALSE to show normally
 * @return TRUE on success, FALSE on failure
 */
extern "C" __declspec(dllexport) BOOL __stdcall SetWindowVisibility(HWND hwnd, BOOL hide) {
    // Validate window handle
    if (hwnd == NULL || !IsWindow(hwnd)) {
        return FALSE;
    }

    DWORD dwAffinity = hide ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
    return SetWindowDisplayAffinity(hwnd, dwAffinity);
}

/**
 * Hide all windows of the current process from screen capture.
 * Windows remain visible to the user but are excluded from screenshots and screen sharing.
 * Only processes valid application windows (visible, top-level, with title).
 */
extern "C" __declspec(dllexport) void __stdcall HideAllWindows() {
    SetAllWindowsVisibilityInternal(TRUE);
}

/**
 * Show all windows of the current process normally.
 * Restores windows to be visible in screenshots and screen sharing.
 */
extern "C" __declspec(dllexport) void __stdcall ShowAllWindows() {
    SetAllWindowsVisibilityInternal(FALSE);
}

/**
 * Hide or show a window from the taskbar.
 * Note: This completely hides/shows the taskbar icon, not just from capture.
 *
 * @param hwnd Window handle to modify
 * @param hide TRUE to hide from taskbar, FALSE to show in taskbar
 * @return TRUE on success, FALSE on failure
 */
extern "C" __declspec(dllexport) BOOL __stdcall HideFromTaskbar(HWND hwnd, BOOL hide) {
    // Validate window handle
    if (hwnd == NULL || !IsWindow(hwnd)) {
        return FALSE;
    }

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (style == 0) {
        return FALSE;
    }

    if (hide) {
        // Hide from taskbar: add TOOLWINDOW style, remove APPWINDOW style
        style |= WS_EX_TOOLWINDOW;
        style &= ~WS_EX_APPWINDOW;
    } else {
        // Show in taskbar: add APPWINDOW style, remove TOOLWINDOW style
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
