#pragma once

#include <vector>
#include <d3d11.h>
#include <dxgi1_2.h>

// BGRA U8 Bitmap
struct Bitmap {
	int                  Width  = 0;
	int                  Height = 0;
	std::vector<uint8_t> Buf;
};


class WinDesktopDup {
public:
	Bitmap Latest;
	BOOL Enabled = false;

	~WinDesktopDup();

	BOOL Initialize();
	void  Close();
	int		 CapturesCount() { return DeskDupls.size(); }
	HBITMAP  CaptureNext(int index);

private:
	ID3D11Device*           D3DDevice        = nullptr;
	ID3D11DeviceContext*    D3DDeviceContext = nullptr;
	void                    Reinitialize();
	std::vector<bool> HaveFrameLocks;
	std::vector<DXGI_OUTPUT_DESC> OutputDescs;
	std::vector<IDXGIOutputDuplication*> DeskDupls;
	HBITMAP GetHBITMAP(int index);
};