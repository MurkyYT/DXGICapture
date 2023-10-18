// DXGICapture.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "DXGICapture.h"
#include "WinDesktopDup.h"
WinDesktopDup* dup ;
extern "C" {
    // This is an example of an exported function.
    DXGICAPTURE_API bool InitCapture(void)
    {
        /*intptr_t ptr = (intptr_t)(new WinDesktopDup());
        ((WinDesktopDup*)ptr)->Initialize();
        return ptr;*/
        try {
            dup = new WinDesktopDup();
            dup->Initialize();
        }
        catch (char* e) { return false; }
        return true;
    }
    DXGICAPTURE_API void DeInitCapture(void) {
        __try {
            if (dup != NULL) {
                delete dup;
                dup = NULL;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dup = NULL;
        }
    }
    DXGICAPTURE_API HBITMAP UpdateFrame(void) {
        __try {
            if (dup != NULL)
                return dup->CaptureNext();
            return NULL;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return NULL;
        }
    }
    DXGICAPTURE_API BOOL IsEnabled(void)
    {
        return dup != nullptr;
    }
}