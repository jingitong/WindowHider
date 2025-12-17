# WindowHider

[English](#) | [简体中文](./README.zh-CN.md)

A Windows DLL library for hiding windows from screen capture, screenshots, and screen sharing while keeping them visible to the user.

## Features

- Hide windows from screenshots, screen recording, and screen sharing
- Windows remain visible and usable to the user
- Support for both x86 and x64 architectures
- Simple C-style exported functions for easy integration


## System Requirements

- Windows 10 v2004 (Build 19041) or higher
- On older Windows versions, windows will show as black boxes instead of being hidden

## Building

### Using Visual Studio

1. Open `WindowHider.sln`
2. Select configuration:
   - **Release | x64** - Build 64-bit DLL
   - **Release | Win32** - Build 32-bit DLL
3. Build the project

### Output Files

- `x64/Release/WindowHider.dll` - 64-bit version
- `Release/WindowHider_32bit.dll` - 32-bit version

## API Reference

### Exported Functions

| Function | Description |
|----------|-------------|
| `SetWindowVisibility(HWND hwnd, BOOL hide)` | Set visibility of a specific window |
| `HideAllWindows()` | Hide all windows of current process |
| `ShowAllWindows()` | Show all windows of current process |
| `HideFromTaskbar(HWND hwnd, BOOL hide)` | Hide/show window from taskbar |

### Function Details

#### SetWindowVisibility
```c
BOOL __stdcall SetWindowVisibility(HWND hwnd, BOOL hide);
```
- `hwnd`: Window handle
- `hide`: `TRUE` to hide, `FALSE` to show
- Returns: `TRUE` on success, `FALSE` on failure

#### HideAllWindows / ShowAllWindows
```c
void __stdcall HideAllWindows();
void __stdcall ShowAllWindows();
```
Automatically enumerates all visible top-level windows of the current process and sets their visibility. **The DLL internally retrieves the calling process's process ID and window handles automatically. Applications simply need to call these functions to hide windows without manually passing process or window information.**

#### HideFromTaskbar
```c
BOOL __stdcall HideFromTaskbar(HWND hwnd, BOOL hide);
```
Controls whether the window appears in the taskbar.

## Usage Examples

### Python Example

```python
import ctypes
import platform
import os

def load_window_hider_dll(dll_directory="."):
    """
    Load WindowHider DLL
    Automatically selects the correct DLL version based on Python architecture
    """
    is_64bit = platform.architecture()[0] == "64bit"
    dll_name = "WindowHider.dll" if is_64bit else "WindowHider_32bit.dll"
    dll_path = os.path.join(dll_directory, dll_name)

    if not os.path.exists(dll_path):
        raise FileNotFoundError(f"DLL not found: {dll_path}")

    dll = ctypes.WinDLL(dll_path)

    # Set function signatures
    dll.SetWindowVisibility.argtypes = [ctypes.c_void_p, ctypes.c_int]
    dll.SetWindowVisibility.restype = ctypes.c_int

    dll.HideAllWindows.argtypes = []
    dll.HideAllWindows.restype = None

    dll.ShowAllWindows.argtypes = []
    dll.ShowAllWindows.restype = None

    dll.HideFromTaskbar.argtypes = [ctypes.c_void_p, ctypes.c_int]
    dll.HideFromTaskbar.restype = ctypes.c_int

    return dll

# Usage example
if __name__ == "__main__":
    import tkinter as tk

    dll = load_window_hider_dll()
    root = tk.Tk()
    root.title("Test Window")
    root.geometry("400x300")

    # Get window handle
    user32 = ctypes.windll.user32
    hwnd = user32.GetAncestor(root.winfo_id(), 2)  # GA_ROOT = 2

    is_hidden = False

    def toggle():
        global is_hidden
        if is_hidden:
            dll.SetWindowVisibility(hwnd, False)
            btn.config(text="Hide Window")
            is_hidden = False
        else:
            dll.SetWindowVisibility(hwnd, True)
            btn.config(text="Show Window")
            is_hidden = True

    btn = tk.Button(root, text="Hide Window", command=toggle)
    btn.pack(pady=50)
    root.mainloop()
```

### C/C++ Example

```cpp
#include <Windows.h>

typedef BOOL(__stdcall* SetWindowVisibilityFunc)(HWND, BOOL);
typedef void(__stdcall* HideAllWindowsFunc)();
typedef void(__stdcall* ShowAllWindowsFunc)();

int main() {
    HMODULE hDll = LoadLibraryW(L"WindowHider.dll");
    if (!hDll) return 1;

    auto SetWindowVisibility = (SetWindowVisibilityFunc)GetProcAddress(hDll, "SetWindowVisibility");
    auto HideAllWindows = (HideAllWindowsFunc)GetProcAddress(hDll, "HideAllWindows");
    auto ShowAllWindows = (ShowAllWindowsFunc)GetProcAddress(hDll, "ShowAllWindows");

    // Hide specific window
    HWND hwnd = FindWindowW(NULL, L"Window Title");
    if (hwnd) {
        SetWindowVisibility(hwnd, TRUE);  // Hide
        // ...
        SetWindowVisibility(hwnd, FALSE); // Show
    }

    // Or hide all windows of current process
    HideAllWindows();
    // ...
    ShowAllWindows();

    FreeLibrary(hDll);
    return 0;
}
```

### Flutter/Dart Example (using ffi)

```dart
import 'dart:ffi';
import 'dart:io';

typedef SetWindowVisibilityNative = Int32 Function(Pointer<Void> hwnd, Int32 hide);
typedef SetWindowVisibilityDart = int Function(Pointer<Void> hwnd, int hide);

typedef HideAllWindowsNative = Void Function();
typedef HideAllWindowsDart = void Function();

class WindowHider {
  late DynamicLibrary _lib;
  late SetWindowVisibilityDart setWindowVisibility;
  late HideAllWindowsDart hideAllWindows;
  late HideAllWindowsDart showAllWindows;

  WindowHider() {
    final dllName = _is64Bit() ? 'WindowHider.dll' : 'WindowHider_32bit.dll';
    _lib = DynamicLibrary.open(dllName);

    setWindowVisibility = _lib
        .lookupFunction<SetWindowVisibilityNative, SetWindowVisibilityDart>(
            'SetWindowVisibility');

    hideAllWindows = _lib
        .lookupFunction<HideAllWindowsNative, HideAllWindowsDart>(
            'HideAllWindows');

    showAllWindows = _lib
        .lookupFunction<HideAllWindowsNative, HideAllWindowsDart>(
            'ShowAllWindows');
  }

  bool _is64Bit() {
    return sizeOf<IntPtr>() == 8;
  }
}
```

## Testing

The project includes a Python test program `test_window_hider.py` for:
- Verifying the DLL works correctly
- Learning how to call DLL functions
- Testing window hide/show functionality

### Running Tests

1. Copy the compiled `WindowHider.dll` (64-bit) or `WindowHider_32bit.dll` (32-bit) to the project root directory
2. Run the test program:

```bash
python test_window_hider.py
```

3. Click the "Hide Window" button to test functionality
4. Use screenshot tools to verify the window is excluded from captures

## How It Works

WindowHider uses the Windows API [SetWindowDisplayAffinity](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity) to set windows to `WDA_EXCLUDEFROMCAPTURE`, excluding them from screen capture.

## Notes

1. **Windows must belong to current process** - `SetWindowDisplayAffinity` can only operate on windows owned by the current process
2. **Correct window handle required** - Must obtain the top-level window handle, not child control handles


## License

MIT License
