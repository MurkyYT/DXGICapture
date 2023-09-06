// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DXGICAPTURE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DXGICAPTURE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DXGICAPTURE_EXPORTS
#define DXGICAPTURE_API __declspec(dllexport)
#else
#define DXGICAPTURE_API __declspec(dllimport)
#endif
extern "C" {
	DXGICAPTURE_API intptr_t InitCapture(void);
	DXGICAPTURE_API void DeInitCapture(intptr_t ptr);
	DXGICAPTURE_API bool UpdateFrame(intptr_t ptr);
	DXGICAPTURE_API HBITMAP GetBitmap(intptr_t ptr);
}