"""
WindowHider DLL 测试程序 - 使用 SetWindowVisibility 直接操作指定窗口
"""

import tkinter as tk
from tkinter import messagebox
import ctypes
from ctypes import wintypes
import os

# Windows API
user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

# SetWindowDisplayAffinity 常量
WDA_NONE = 0x00000000
WDA_MONITOR = 0x00000001
WDA_EXCLUDEFROMCAPTURE = 0x00000011

class WindowHiderTest:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("WindowHider 测试程序")
        self.root.geometry("450x400")

        self.is_hidden = False
        self.dll = None
        self.hwnd = None

        self.setup_ui()
        self.load_dll()

        # 获取窗口句柄（需要在窗口显示后获取）
        self.root.after(100, self.get_window_handle)

    def get_window_handle(self):
        """获取当前窗口的顶层句柄"""
        self.root.update_idletasks()

        # 方法1: 使用 GetAncestor 获取根窗口
        GA_ROOT = 2
        child_hwnd = self.root.winfo_id()
        self.hwnd = user32.GetAncestor(child_hwnd, GA_ROOT)

        if self.hwnd == 0:
            # 方法2: 使用 GetParent 循环向上查找
            self.hwnd = child_hwnd
            while True:
                parent = user32.GetParent(self.hwnd)
                if parent == 0:
                    break
                self.hwnd = parent

        print(f"子窗口句柄: {child_hwnd} (0x{child_hwnd:X})")
        print(f"顶层窗口句柄: {self.hwnd} (0x{self.hwnd:X})")
        self.hwnd_var.set(f"顶层窗口句柄: 0x{self.hwnd:X}")

    def setup_ui(self):
        # 标题
        title_label = tk.Label(self.root, text="WindowHider 功能测试", font=("Arial", 16, "bold"))
        title_label.pack(pady=15)

        # 窗口句柄显示
        self.hwnd_var = tk.StringVar(value="窗口句柄: 获取中...")
        hwnd_label = tk.Label(self.root, textvariable=self.hwnd_var, font=("Arial", 10))
        hwnd_label.pack(pady=5)

        # 状态显示
        self.status_var = tk.StringVar(value="状态：正常显示")
        self.status_label = tk.Label(self.root, textvariable=self.status_var, font=("Arial", 12))
        self.status_label.pack(pady=10)

        # 说明文字
        info_text = "点击按钮切换隐藏状态\n隐藏后：你仍能看到窗口，但截图/共享中不可见"
        info_label = tk.Label(self.root, text=info_text, font=("Arial", 10))
        info_label.pack(pady=10)

        # 切换按钮
        self.toggle_btn = tk.Button(
            self.root,
            text="隐藏窗口 (从截图中)",
            font=("Arial", 12),
            width=25,
            height=2,
            command=self.toggle_visibility
        )
        self.toggle_btn.pack(pady=15)

        # 结果显示
        self.result_var = tk.StringVar(value="")
        result_label = tk.Label(self.root, textvariable=self.result_var, font=("Arial", 10))
        result_label.pack(pady=5)

        # DLL 状态
        self.dll_status_var = tk.StringVar(value="DLL: 未加载")
        dll_status_label = tk.Label(self.root, textvariable=self.dll_status_var, font=("Arial", 9))
        dll_status_label.pack(side="bottom", pady=5)

    def load_dll(self):
        """加载 WindowHider.dll"""
        dll_paths = [
            "WindowHider.dll",
            "Build/bin/Release/WindowHider.dll",
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "WindowHider.dll"),
        ]

        for path in dll_paths:
            if os.path.exists(path):
                try:
                    self.dll = ctypes.WinDLL(os.path.abspath(path))

                    # 设置函数签名 - 使用 SetWindowVisibility 直接操作指定窗口
                    self.dll.SetWindowVisibility.argtypes = [wintypes.HWND, wintypes.BOOL]
                    self.dll.SetWindowVisibility.restype = wintypes.BOOL

                    self.dll_status_var.set(f"DLL: 已加载")
                    print(f"DLL 加载成功: {path}")
                    return
                except Exception as e:
                    print(f"加载 {path} 失败: {e}")

        self.dll_status_var.set("DLL: 未找到")
        self.toggle_btn.config(state="disabled")
        print("DLL 未找到")

    def toggle_visibility(self):
        """切换窗口可见性 - 使用 HideAllWindows/ShowAllWindows"""
        if not self.dll:
            messagebox.showerror("错误", "DLL 未加载")
            return

        try:
            if self.is_hidden:
                # 显示窗口 - 使用 DLL 的 ShowAllWindows
                self.dll.ShowAllWindows()
                self.is_hidden = False
                self.status_var.set("状态：正常显示")
                self.toggle_btn.config(text="隐藏窗口 (从截图中)")
                self.result_var.set("已调用 ShowAllWindows()")
                print("调用 ShowAllWindows()")
            else:
                # 隐藏窗口 - 使用 DLL 的 HideAllWindows
                self.dll.HideAllWindows()
                self.is_hidden = True
                self.status_var.set("状态：已隐藏（截图不可见，你仍可见）")
                self.toggle_btn.config(text="显示窗口 (恢复正常)")
                self.result_var.set("已调用 HideAllWindows()")
                print("调用 HideAllWindows()")
        except Exception as e:
            print(f"操作失败: {e}")
            messagebox.showerror("错误", f"操作失败: {e}")

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    print("启动测试程序...")
    print(f"Python 进程 ID: {os.getpid()}")
    app = WindowHiderTest()
    app.run()

