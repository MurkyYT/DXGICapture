#include <windows.h>
#include <dxgi.h>

#ifdef DXGICAPTURE_EXPORTS
#define DXGICAPTURE_API __declspec(dllexport)
#else
#define DXGICAPTURE_API __declspec(dllimport)
#endif

#define DXGICAPTURE_ALL_SCREENS -1

#ifdef __cplusplus
extern "C" {
#endif

DXGICAPTURE_API BOOL             DXGI_InitCapture(void);
DXGICAPTURE_API void             DXGI_SetTimeout(UINT ms);
DXGICAPTURE_API void             DXGI_DeInitCapture(void);
DXGICAPTURE_API HBITMAP          DXGI_CaptureScreen(int index);
DXGICAPTURE_API DXGI_OUTPUT_DESC DXGI_GetOutputDescription(int index);
DXGICAPTURE_API HBITMAP          DXGI_UpdateFrame(void);
DXGICAPTURE_API int              DXGI_OutputsCount(void);
DXGICAPTURE_API BOOL             DXGI_IsEnabled(void);

#ifdef __cplusplus
}
#endif