// DXGICapture.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "DXGICapture.h"
#include "WinDesktopDup.h"
WinDesktopDup dup;
extern "C" {
    // This is an example of an exported function.
    DXGICAPTURE_API bool InitCapture(void)
    {
        /*intptr_t ptr = (intptr_t)(new WinDesktopDup());
        ((WinDesktopDup*)ptr)->Initialize();
        return ptr;*/
        try {
            dup.Initialize();
        }
        catch (char* e) { return false; }
        return true;
    }
    DXGICAPTURE_API void DeInitCapture(void) {
        dup.Close();
    }
    DXGICAPTURE_API HBITMAP UpdateFrame(void) {
        if (dup.Enabled)
            return dup.CaptureNext();
        return NULL;
    }
    DXGICAPTURE_API BOOL IsEnabled(void)
    {
        return dup.Enabled;
    }
}