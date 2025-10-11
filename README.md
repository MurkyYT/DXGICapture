## DXGICapture
C++ Library with C exports to capture screen with Windows Desktop Duplication API

**Original code from [WindowsDesktopDuplicationSample](https://github.com/bmharper/WindowsDesktopDuplicationSample)!**

This library is used by [CSAuto](https://github.com/MurkyYT/CSAuto) to capture the screen of [CS2](https://store.steampowered.com/app/730/CounterStrike_2/)

### Features
- Multi-monitor support
- Rotation handling
- C exports for easy interop with C# and other languages

### System Requirements
- Windows 8 or later
- DXGI 1.2+ compatible GPU
- DirectX 11 support

### Build Instructions
1. Open the solution in Visual Studio 2022
2. Select your target platform (x64 or x86)
3. Build the project (Release or Debug configuration)
4. The compiled DLL will be in the `bin` folder

### Usage

**C#**

Example DXGIcapture.cs class in [here](https://github.com/MurkyYT/CSAuto/blob/dev/src/CSAuto/Utils/DXGICapture.cs)

```C#
// Create new DXGICapture object
DXGICapture DXGIcapture = new DXGICapture();
// Initialize the capture
DXGIcapture.Init();
// Get pointer to the bitmap
IntPtr _handle = DXGIcapture.GetCapture();
// Check for invalid capture
if (_handle == IntPtr.Zero)
{
    DXGIcapture.DeInit();
    DXGIcapture.Init();
    _handle = DXGIcapture.GetCapture();
}
using (Bitmap bitmap = Image.FromHbitmap(_handle))
{
    // Do stuff with bitmap
}
// Deinitialize capture
DXGIcapture.DeInit();
```

### Thread Safety
**The C++ library is NOT thread-safe.** All methods must be called from a single thread or externally synchronized. The C# wrapper provides thread-safety through locking.

### License
MIT License - See [LICENSE](LICENSE) file for details

Copyright (c) 2025 MurkyYT