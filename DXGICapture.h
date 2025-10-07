#include <windows.h>

#ifdef DXGICAPTURE_EXPORTS
#define DXGICAPTURE_API __declspec(dllexport)
#else
#define DXGICAPTURE_API __declspec(dllimport)
#endif

#define DXGICAPTURE_ALL_SCREENS -1

extern "C" {
	DXGICAPTURE_API BOOL InitCapture(void);
	DXGICAPTURE_API void DeInitCapture(void);
	DXGICAPTURE_API HBITMAP CaptureScreen(int index);
	DXGICAPTURE_API HBITMAP UpdateFrame(void);
	DXGICAPTURE_API int OutputsCount(void);
	DXGICAPTURE_API BOOL IsEnabled(void);
}