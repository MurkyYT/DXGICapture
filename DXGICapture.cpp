#include "DXGICapture.h"
#include "WinDesktopDup.h"

// Windows Desktop Duplication dll member
WinDesktopDup dup;

extern "C" {
    DXGICAPTURE_API BOOL InitCapture(void)
    {
        return dup.Initialize();
    }
    DXGICAPTURE_API int OutputsCount(void)
    {
        return dup.CapturesCount();
    }
    DXGICAPTURE_API void DeInitCapture(void) {
        dup.Close();
    }
    DXGICAPTURE_API HBITMAP UpdateFrame(void) {
        if (dup.Enabled)
            return dup.CaptureNext(0);
        return NULL;
    }
    DXGICAPTURE_API HBITMAP CaptureScreen(int index) {
        if (dup.Enabled)
            return dup.CaptureNext(index);
        return NULL;
    }
    DXGICAPTURE_API BOOL IsEnabled(void)
    {
        return dup.Enabled;
    }
}