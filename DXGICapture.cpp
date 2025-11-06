#include "DXGICapture.h"
#include "WinDesktopDup.h"

// Windows Desktop Duplication dll member
WinDesktopDup dup;

extern "C" {

    DXGICAPTURE_API void DXGI_SetTimeout(UINT ms)
    {
        dup.SetTimeout(ms);
    }

    DXGICAPTURE_API BOOL DXGI_InitCapture(void)
    {
        return dup.Initialize();
    }

    DXGICAPTURE_API int DXGI_OutputsCount(void)
    {
        return static_cast<int>(dup.CapturesCount());
    }

    DXGICAPTURE_API int DXGI_AdaptersCount(void)
    {
        return static_cast<int>(dup.AdaptersCount());
    }

    DXGICAPTURE_API void DXGI_GetOutputDescription(int index, DXGI_OUTPUT_DESC* desc)
    {
        if (!desc) return;
        *desc = dup.GetOutputDescription(index);
    }

    DXGICAPTURE_API void DXGI_GetAdapterDescription(int index, DXGI_ADAPTER_DESC1* desc)
    {
        if (!desc) return;
        *desc = dup.GetAdapterDescription(index);
    }

    DXGICAPTURE_API void DXGI_DeInitCapture(void)
    {
        dup.Close();
    }

    DXGICAPTURE_API HBITMAP DXGI_UpdateFrame(void)
    {
        if (dup.IsEnabled())
            return dup.CaptureNext(0);
        return NULL;
    }

    DXGICAPTURE_API HBITMAP DXGI_CaptureScreen(int index)
    {
        if (dup.IsEnabled())
            return dup.CaptureNext(index);
        return NULL;
    }

    DXGICAPTURE_API BOOL DXGI_IsEnabled(void)
    {
        return dup.IsEnabled();
    }
}
