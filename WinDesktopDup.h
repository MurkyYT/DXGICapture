#pragma once

#include <vector>
#include <unordered_map>
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
	~WinDesktopDup();

	BOOL IsEnabled() { return m_enabled; }
	BOOL Initialize();
	void  Close();
	int		 CapturesCount() { return m_deskDupls.size(); }
	HBITMAP  CaptureNext(int index);

private:
	void                    Reinitialize();
	std::vector<bool> m_haveFrameLocks;
	std::vector<DXGI_OUTPUT_DESC> m_outputDescs;
	std::vector<IDXGIOutputDuplication*> m_deskDupls;
	std::vector<ID3D11Device*> m_devices;
	std::vector<ID3D11DeviceContext*> m_deviceContexts;
	std::unordered_map<IDXGIOutputDuplication*, UINT> m_deskDuplToDeviceIndex;
	HBITMAP GetHBITMAP(int index);
	Bitmap m_latestBmp;
	BOOL m_enabled = false;
};