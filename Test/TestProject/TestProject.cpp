#include <windows.h>
#include <gdiplus.h>
#include <string>

#pragma comment(lib, "gdiplus.lib")
#include "../../DXGICapture.h"

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;
    UINT size = 0;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR gdiplusToken;

int main()
{
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	DXGI_InitCapture();

    Sleep(100);

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);

    HBITMAP hBitmap = DXGI_CaptureScreen(DXGICAPTURE_ALL_SCREENS);
	Gdiplus::Bitmap bitmap(hBitmap, nullptr);
	bitmap.Save(L"./test_all.png", &pngClsid, nullptr);

    printf("Saved test_all.png to CWD\n");

    DeleteObject(hBitmap);

    for (int i = 0; i < DXGI_OutputsCount(); i++)
    {
        HBITMAP hBitmap = DXGI_CaptureScreen(i);

        Gdiplus::Bitmap bitmap(hBitmap, nullptr);
        bitmap.Save((std::wstring(L"./test") + std::to_wstring(i) + L".png").c_str(), &pngClsid, nullptr);

        printf("Saved test%d.png to CWD\n", i);

        DeleteObject(hBitmap);
    }

    hBitmap = DXGI_UpdateFrame();

    Gdiplus::Bitmap bitmap2(hBitmap, nullptr);
    bitmap2.Save(L"./test_old.png", &pngClsid, nullptr);

    printf("Saved test_old.png to CWD\n");

    DeleteObject(hBitmap);
    
    DXGI_DeInitCapture();
}