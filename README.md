# WindowHider

WindowHider 是一个 Windows DLL 库，用于将窗口从屏幕捕获、截图和屏幕共享中隐藏。

## 功能特性

- 使窗口对截图、屏幕录制、屏幕共享不可见
- 用户本人仍可正常看到和使用窗口
- 支持 x86 和 x64 架构
- 提供简单的 C 风格导出函数，易于集成

## 系统要求

- Windows 10 v2004 (Build 19041) 或更高版本
- 在较低版本的 Windows 上，窗口会显示为黑色而非完全隐藏

## 编译

### 使用 Visual Studio

1. 打开 `WindowHider.sln`
2. 选择配置：
   - **Release | x64** - 生成 64 位 DLL
   - **Release | Win32** - 生成 32 位 DLL
3. 编译项目

### 输出文件

- `x64/Release/WindowHider.dll` - 64 位版本
- `Release/WindowHider_32bit.dll` - 32 位版本

## API 参考

### 导出函数

| 函数 | 说明 |
|------|------|
| `SetWindowVisibility(HWND hwnd, BOOL hide)` | 设置指定窗口的可见性 |
| `HideAllWindows()` | 隐藏当前进程的所有窗口 |
| `ShowAllWindows()` | 显示当前进程的所有窗口 |
| `HideFromTaskbar(HWND hwnd, BOOL hide)` | 从任务栏隐藏/显示窗口 |

### 函数详解

#### SetWindowVisibility
```c
BOOL __stdcall SetWindowVisibility(HWND hwnd, BOOL hide);
```
- `hwnd`: 窗口句柄
- `hide`: `TRUE` 隐藏窗口，`FALSE` 显示窗口
- 返回值: 成功返回 `TRUE`，失败返回 `FALSE`

#### HideAllWindows / ShowAllWindows
```c
void __stdcall HideAllWindows();
void __stdcall ShowAllWindows();
```
自动遍历当前进程的所有可见顶层窗口并设置其可见性。

#### HideFromTaskbar
```c
BOOL __stdcall HideFromTaskbar(HWND hwnd, BOOL hide);
```
控制窗口是否在任务栏中显示。

## 使用示例

### Python 示例

```python
import ctypes
import platform
import os

# 根据系统架构加载对应的 DLL
def load_window_hider_dll(dll_directory="."):
    """
    加载 WindowHider DLL
    自动根据 Python 解释器的架构选择正确的 DLL 版本
    """
    # 检查 Python 解释器是 32 位还是 64 位
    is_64bit = platform.architecture()[0] == "64bit"

    if is_64bit:
        dll_name = "WindowHider.dll"
    else:
        dll_name = "WindowHider_32bit.dll"

    dll_path = os.path.join(dll_directory, dll_name)

    if not os.path.exists(dll_path):
        raise FileNotFoundError(f"找不到 DLL: {dll_path}")

    # 加载 DLL
    dll = ctypes.WinDLL(dll_path)

    # 设置函数签名
    dll.SetWindowVisibility.argtypes = [ctypes.c_void_p, ctypes.c_int]
    dll.SetWindowVisibility.restype = ctypes.c_int

    dll.HideAllWindows.argtypes = []
    dll.HideAllWindows.restype = None

    dll.ShowAllWindows.argtypes = []
    dll.ShowAllWindows.restype = None

    dll.HideFromTaskbar.argtypes = [ctypes.c_void_p, ctypes.c_int]
    dll.HideFromTaskbar.restype = ctypes.c_int

    return dll

# 使用示例
if __name__ == "__main__":
    import tkinter as tk

    # 加载 DLL
    dll = load_window_hider_dll()

    # 创建窗口
    root = tk.Tk()
    root.title("测试窗口")
    root.geometry("400x300")

    # 获取窗口句柄
    user32 = ctypes.windll.user32
    hwnd = user32.GetAncestor(root.winfo_id(), 2)  # GA_ROOT = 2

    is_hidden = False

    def toggle():
        global is_hidden
        if is_hidden:
            result = dll.SetWindowVisibility(hwnd, False)
            btn.config(text="隐藏窗口")
            is_hidden = False
        else:
            result = dll.SetWindowVisibility(hwnd, True)
            btn.config(text="显示窗口")
            is_hidden = True
        print(f"操作结果: {result}")

    btn = tk.Button(root, text="隐藏窗口", command=toggle)
    btn.pack(pady=50)

    root.mainloop()
```

### C/C++ 示例

```cpp
#include <Windows.h>

// 函数指针类型定义
typedef BOOL(__stdcall* SetWindowVisibilityFunc)(HWND, BOOL);
typedef void(__stdcall* HideAllWindowsFunc)();
typedef void(__stdcall* ShowAllWindowsFunc)();

int main() {
    // 加载 DLL
    HMODULE hDll = LoadLibraryW(L"WindowHider.dll");
    if (!hDll) {
        return 1;
    }

    // 获取函数指针
    auto SetWindowVisibility = (SetWindowVisibilityFunc)GetProcAddress(hDll, "SetWindowVisibility");
    auto HideAllWindows = (HideAllWindowsFunc)GetProcAddress(hDll, "HideAllWindows");
    auto ShowAllWindows = (ShowAllWindowsFunc)GetProcAddress(hDll, "ShowAllWindows");

    // 隐藏指定窗口
    HWND hwnd = FindWindowW(NULL, L"窗口标题");
    if (hwnd) {
        SetWindowVisibility(hwnd, TRUE);  // 隐藏
        // ...
        SetWindowVisibility(hwnd, FALSE); // 显示
    }

    // 或者隐藏当前进程所有窗口
    HideAllWindows();
    // ...
    ShowAllWindows();

    FreeLibrary(hDll);
    return 0;
}
```

### Flutter/Dart 示例 (使用 ffi)

```dart
import 'dart:ffi';
import 'dart:io';

// 定义函数签名
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
    // 根据系统架构加载对应 DLL
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
    // Dart 在 64 位系统上 sizeOf<IntPtr>() == 8
    return sizeOf<IntPtr>() == 8;
  }
}
```

## 测试

项目包含一个 Python 测试程序 `test_window_hider.py`，可用于：

- 验证 DLL 是否正常工作
- 查看如何调用 DLL 函数
- 测试窗口隐藏/显示效果

### 运行测试

1. 将编译好的 `WindowHider.dll`（64位）或 `WindowHider_32bit.dll`（32位）复制到项目根目录
2. 运行测试程序：

```bash
python test_window_hider.py
```

3. 点击"隐藏窗口"按钮测试功能
4. 使用截图工具验证窗口是否从截图中消失

## 工作原理

WindowHider 使用 Windows API [SetWindowDisplayAffinity](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity) 将窗口设置为 `WDA_EXCLUDEFROMCAPTURE`，使其从屏幕捕获中排除。

## 注意事项

1. **窗口必须属于当前进程** - `SetWindowDisplayAffinity` 只能操作当前进程拥有的窗口
2. **需要正确的窗口句柄** - 必须获取顶层窗口的句柄，而非子控件句柄
3. **不是安全功能** - 这不能防止物理截屏（如手机拍照）

## 许可证

MIT License
