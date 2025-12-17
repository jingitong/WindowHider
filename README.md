# WindowHider

[English](#) | [简体中文](./README.zh-CN.md)

A Windows DLL library for hiding windows from screen capture, screenshots, and screen sharing while keeping them visible to the user.

## Features

- Hide windows from screenshots, screen recording, and screen sharing
- Windows remain visible and usable to the user
- Support for both x86 and x64 architectures
- Simple C-style exported functions for easy integration
- Production-ready with robust error handling

## System Requirements

- Windows 10 v2004 (Build 19041) or higher
- On older Windows versions, windows will show as black boxes instead of being hidden

## Testing

The project includes a Python test program `test_window_hider.py` for:
- Verifying the DLL works correctly
- Learning how to call DLL functions
- Testing window hide/show functionality

Run the test:
```bash
python test_window_hider.py
```

## How It Works

WindowHider uses the Windows API [SetWindowDisplayAffinity](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity) to set windows to `WDA_EXCLUDEFROMCAPTURE`, excluding them from screen capture.

## Notes

1. **Windows must belong to current process** - `SetWindowDisplayAffinity` can only operate on windows owned by the current process
2. **Correct window handle required** - Must obtain the top-level window handle, not child control handles
3. **Not a security feature** - This cannot prevent physical screenshots (e.g., phone cameras)

## License

MIT License
