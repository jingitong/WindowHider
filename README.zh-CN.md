# WindowHider

[English](./README.md) | [简体中文](#)

WindowHider 是一个 Windows DLL 库，用于将窗口从屏幕捕获、截图和屏幕共享中隐藏。

## 功能特性

- 使窗口对截图、屏幕录制、屏幕共享不可见
- 用户本人仍可正常看到和使用窗口
- 支持 x86 和 x64 架构
- 提供简单的 C 风格导出函数，易于集成
- 生产级代码，具有完善的错误处理

## 系统要求

- Windows 10 v2004 (Build 19041) 或更高版本
- 在较低版本的 Windows 上，窗口会显示为黑色而非完全隐藏

## 测试

项目包含一个 Python 测试程序 	est_window_hider.py，可用于：
- 验证 DLL 是否正常工作
- 查看如何调用 DLL 函数
- 测试窗口隐藏/显示效果

运行测试：
`ash
python test_window_hider.py
`

## 工作原理

WindowHider 使用 Windows API [SetWindowDisplayAffinity](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity) 将窗口设置为 WDA_EXCLUDEFROMCAPTURE，使其从屏幕捕获中排除。

## 注意事项

1. **窗口必须属于当前进程** - SetWindowDisplayAffinity 只能操作当前进程拥有的窗口
2. **需要正确的窗口句柄** - 必须获取顶层窗口的句柄，而非子控件句柄
3. **不是安全功能** - 这不能防止物理截屏（如手机拍照）

## 许可证

MIT License
