// DXGICapture.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "DXGICapture.h"
#include "WinDesktopDup.h"
extern "C" {
    // This is an example of an exported function.
    DXGICAPTURE_API intptr_t InitCapture(void)
    {
        intptr_t ptr = (intptr_t)(new WinDesktopDup());
        ((WinDesktopDup*)ptr)->Initialize();
        return ptr;
    }
    DXGICAPTURE_API void DeInitCapture(intptr_t ptr) {
        delete (WinDesktopDup*)ptr;
    }
    DXGICAPTURE_API bool UpdateFrame(intptr_t ptr) {
        return ((WinDesktopDup*)ptr)->CaptureNext();
    }
    DXGICAPTURE_API HBITMAP GetBitmap(intptr_t ptr) {
        return ((WinDesktopDup*)ptr)->GetHBITMAP();
    }
}