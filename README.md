## Windows Desktop Duplication API Library
**Original code from [WindowsDesktopDuplicationSample](https://github.com/bmharper/WindowsDesktopDuplicationSample)!**

This library is used by [CSAuto](https://github.com/MurkyYT/CSAuto) to capture the screen of [CS2](https://store.steampowered.com/app/730/CounterStrike_2/)
### Usage
**C#**
Example DXGIcapture.cs class in [here](https://github.com/MurkyYT/CSAuto/blob/dev/src/CSAuto/Utils/DXGICapture.cs)
```C#
//Create new DXGICapture object
DXGICapture DXGIcapture = new DXGICapture();
//Initialize the capture
DXGIcapture.Init();
//Get pointer to the bitmap
IntPtr _handle = DXGIcapture.GetCapture();
//Check for invalid capture
if (_handle == IntPtr.Zero)
{
    DXGIcapture.DeInit();
    DXGIcapture.Init();
    _handle = DXGIcapture.GetCapture();
}
using (Bitmap bitmap = Image.FromHbitmap(_handle))
{
//Do stuff with bitmap
}
//Deinitialize capture
DXGIcapture.DeInit();
```
