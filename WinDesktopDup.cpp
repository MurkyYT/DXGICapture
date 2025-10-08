#include "WinDesktopDup.h"

WinDesktopDup::~WinDesktopDup() {
	Close();
}

BOOL WinDesktopDup::Initialize() {
	HDESK hDesk = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hDesk)
		return FALSE;

	BOOL deskAttached = SetThreadDesktop(hDesk) != 0;
	CloseDesktop(hDesk);
	hDesk = NULL;
	if (!deskAttached) {
		DWORD error = GetLastError();
		if (error != ERROR_BUSY)
			return FALSE;
	}

	HRESULT hr = S_OK;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1 };

	size_t numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;

	IDXGIFactory1* dxgiFactory = NULL;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
	if (FAILED(hr))
		return FALSE;

	std::vector<IDXGIAdapter1*> adapters;
	IDXGIAdapter1* adapter = NULL;
	for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		if (adapter) {
			adapters.push_back(adapter);
		}
	}

	dxgiFactory->Release();
	dxgiFactory = NULL;

	if (adapters.empty())
		return FALSE;

	for (size_t adapterIdx = 0; adapterIdx < adapters.size(); adapterIdx++) {
		IDXGIAdapter1* currentAdapter = adapters[adapterIdx];

		ID3D11Device* d3dDevice = NULL;
		ID3D11DeviceContext* d3dDeviceContext = NULL;

		hr = D3D11CreateDevice(currentAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, featureLevels, (UINT)numFeatureLevels,
			D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &d3dDeviceContext);

		if (FAILED(hr)) {
			currentAdapter->Release();
			continue;
		}

		bool addedOutput = false;

		IDXGIOutput* dxgiOutput = NULL;
		for (UINT i = 0; currentAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; ++i) {
			if (dxgiOutput) {
				DXGI_OUTPUT_DESC outputDesc;
				dxgiOutput->GetDesc(&outputDesc);

				IDXGIOutput1* dxgiOutput1 = NULL;
				hr = dxgiOutput->QueryInterface(__uuidof(dxgiOutput1), (void**)&dxgiOutput1);
				dxgiOutput->Release();
				dxgiOutput = NULL;

				if (SUCCEEDED(hr)) {
					IDXGIOutputDuplication* deskDupl = NULL;
					hr = dxgiOutput1->DuplicateOutput(d3dDevice, &deskDupl);
					dxgiOutput1->Release();
					dxgiOutput1 = NULL;

					if (SUCCEEDED(hr)) {
						m_outputDescs.push_back(outputDesc);
						m_deskDupls.push_back(deskDupl);
						m_haveFrameLocks.push_back(false);
						m_deskDuplToDeviceIndex[deskDupl] = m_devices.size();
						addedOutput = true;
					}
				}
			}
		}

		if (!addedOutput) {
			if (d3dDeviceContext) {
				d3dDeviceContext->Release();
			}
			if (d3dDevice) {
				d3dDevice->Release();
			}
		}
		else 
		{
			m_devices.push_back(d3dDevice);
			m_deviceContexts.push_back(d3dDeviceContext);
		}

		currentAdapter->Release();
	}

	if (m_deskDupls.empty())
		return FALSE;

	m_enabled = true;
	return TRUE;
}

void WinDesktopDup::Close() {
	for (size_t i = 0; i < m_deskDupls.size(); i++)
	{
		IDXGIOutputDuplication* DeskDupl = m_deskDupls[i];
		if (DeskDupl)
			DeskDupl->Release();
	}

	for (size_t i = 0; i < m_devices.size(); i++) {
		if (m_devices[i])
			m_devices[i]->Release();
		if (m_deviceContexts[i])
			m_deviceContexts[i]->Release();
	}

	m_deskDupls.clear();
	m_haveFrameLocks.clear();
	m_outputDescs.clear();
	m_devices.clear();
	m_deviceContexts.clear();
	m_deskDuplToDeviceIndex.clear();
	m_enabled = false;
}

void WinDesktopDup::Reinitialize()
{
	Close();
	Initialize();
}

HBITMAP WinDesktopDup::CaptureNext(int index) {
	if (m_deskDupls.size() == 0)
		return NULL;

	if (index == -1)
	{
		int leftMostX = 0, rightMostX = 0;
		int topMostY = 0, bottomMostY = 0;

		for (size_t i = 0; i < m_outputDescs.size(); i++)
		{
			if (m_outputDescs[i].DesktopCoordinates.left < leftMostX)
				leftMostX = m_outputDescs[i].DesktopCoordinates.left;

			if (m_outputDescs[i].DesktopCoordinates.right > rightMostX)
				rightMostX = m_outputDescs[i].DesktopCoordinates.right;

			if (m_outputDescs[i].DesktopCoordinates.bottom > bottomMostY)
				bottomMostY = m_outputDescs[i].DesktopCoordinates.bottom;

			if (m_outputDescs[i].DesktopCoordinates.top < topMostY)
				topMostY = m_outputDescs[i].DesktopCoordinates.top;
		}

		int width = rightMostX - leftMostX, height = bottomMostY - topMostY;

		HDC        hdc = GetDC(NULL);
		BITMAPINFO inf;
		memset(&inf, 0, sizeof(inf));
		inf.bmiHeader.biSize = sizeof(inf.bmiHeader);
		inf.bmiHeader.biWidth = width;
		inf.bmiHeader.biHeight = -height;
		inf.bmiHeader.biPlanes = 1;
		inf.bmiHeader.biBitCount = 32;
		inf.bmiHeader.biCompression = BI_RGB;
		void* bits = NULL;
		HBITMAP dib = CreateDIBSection(hdc, &inf, 0, &bits, NULL, 0);

		if (!dib) {
			ReleaseDC(NULL, hdc);
			return NULL;
		}

		HDC hdcDest = CreateCompatibleDC(hdc);
		HDC hdcSrc = CreateCompatibleDC(hdc);
		HBITMAP oldDest = (HBITMAP)SelectObject(hdcDest, dib);

		for (size_t i = 0; i < m_deskDupls.size(); i++)
		{
			int xCord = m_outputDescs[i].DesktopCoordinates.left - leftMostX;
			int yCord = m_outputDescs[i].DesktopCoordinates.top - topMostY;

			int bmpWidth = m_outputDescs[i].DesktopCoordinates.right - m_outputDescs[i].DesktopCoordinates.left;
			int bmpHeight = m_outputDescs[i].DesktopCoordinates.bottom - m_outputDescs[i].DesktopCoordinates.top;

			HBITMAP bmp = CaptureNext((int)i);

			if (bmp) {
				HBITMAP oldSrc = (HBITMAP)SelectObject(hdcSrc, bmp);
				BitBlt(hdcDest, xCord, yCord, bmpWidth, bmpHeight, hdcSrc, 0, 0, SRCCOPY);
				SelectObject(hdcSrc, oldSrc);

				DeleteObject(bmp);
			}
		}

		SelectObject(hdcDest, oldDest);
		DeleteDC(hdcSrc);
		DeleteDC(hdcDest);
		ReleaseDC(NULL, hdc);

		return dib;
	}
	else
	{
		if ((size_t)index >= m_deskDupls.size())
			return NULL;

		HRESULT hr;
		IDXGIOutputDuplication* DeskDupl = m_deskDupls[index];
		auto it = m_deskDuplToDeviceIndex.find(DeskDupl);
		if (it == m_deskDuplToDeviceIndex.end())
			return NULL;
		UINT deviceIndex = it->second;
		if (deviceIndex >= m_devices.size())
			return NULL;
		ID3D11Device* device = m_devices[deviceIndex];
		ID3D11DeviceContext* deviceContext = m_deviceContexts[deviceIndex];

		if (m_haveFrameLocks[index]) {
			m_haveFrameLocks[index] = false;
			hr = DeskDupl->ReleaseFrame();
		}

		IDXGIResource* deskRes = NULL;
		DXGI_OUTDUPL_FRAME_INFO frameInfo;

		hr = DeskDupl->AcquireNextFrame(500, &frameInfo, &deskRes);
		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			return NULL;

		if (FAILED(hr)) {
			Reinitialize();
			return NULL;
		}

		m_haveFrameLocks[index] = true;

		ID3D11Texture2D* gpuTex = NULL;
		hr = deskRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&gpuTex);
		deskRes->Release();
		deskRes = NULL;
		if (FAILED(hr))
			return NULL;

		BOOL ok = true;

		D3D11_TEXTURE2D_DESC desc;
		gpuTex->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		ID3D11Texture2D* cpuTex = NULL;
		hr = device->CreateTexture2D(&desc, NULL, &cpuTex);

		if (SUCCEEDED(hr))
			deviceContext->CopyResource(cpuTex, gpuTex);
		else
			ok = false;

		gpuTex->Release();

		if (!cpuTex)
			return NULL;

		D3D11_MAPPED_SUBRESOURCE sr;
		hr = deviceContext->Map(cpuTex, 0, D3D11_MAP_READ, 0, &sr);
		if (SUCCEEDED(hr)) {
			if (m_latestBmp.Width != desc.Width || m_latestBmp.Height != desc.Height) {
				m_latestBmp.Width = desc.Width;
				m_latestBmp.Height = desc.Height;
				m_latestBmp.Buf.resize(desc.Width * desc.Height * 4);
			}
			if (sr.RowPitch == desc.Width * 4) {
				memcpy(m_latestBmp.Buf.data(), sr.pData, desc.Width * desc.Height * 4);
			}
			else {
				for (int y = 0; y < (int)desc.Height; y++)
					memcpy(m_latestBmp.Buf.data() + y * desc.Width * 4, (uint8_t*)sr.pData + sr.RowPitch * y, desc.Width * 4);
			}
			deviceContext->Unmap(cpuTex, 0);
		}
		else {
			ok = false;
		}

		cpuTex->Release();

		if (!ok)
			return NULL;

		HBITMAP res = GetHBITMAP(index);

		return res;
	}
}

HBITMAP WinDesktopDup::GetHBITMAP(int index) {
	if (m_latestBmp.Buf.empty() || m_latestBmp.Width == 0 || m_latestBmp.Height == 0)
		return NULL;

	HDC hdc = GetDC(NULL);
	BITMAPINFO inf;
	memset(&inf, 0, sizeof(inf));

	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if (index >= 0 && index < (int)m_outputDescs.size()) {
		rotation = m_outputDescs[index].Rotation;
	}

	int finalWidth = m_latestBmp.Width;
	int finalHeight = m_latestBmp.Height;

	if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
		finalWidth = m_latestBmp.Height;
		finalHeight = m_latestBmp.Width;
	}

	inf.bmiHeader.biSize = sizeof(inf.bmiHeader);
	inf.bmiHeader.biWidth = finalWidth;
	inf.bmiHeader.biHeight = -finalHeight;
	inf.bmiHeader.biPlanes = 1;
	inf.bmiHeader.biBitCount = 32;
	inf.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HBITMAP dib = CreateDIBSection(hdc, &inf, 0, &bits, NULL, 0);

	if (!dib) {
		ReleaseDC(NULL, hdc);
		return NULL;
	}

	uint32_t* src = (uint32_t*)m_latestBmp.Buf.data();
	uint32_t* dst = (uint32_t*)bits;

	switch (rotation) {
	case DXGI_MODE_ROTATION_IDENTITY:
	case DXGI_MODE_ROTATION_UNSPECIFIED:
	case DXGI_MODE_ROTATION_ROTATE180:
		memcpy(bits, m_latestBmp.Buf.data(), m_latestBmp.Width * m_latestBmp.Height * 4);
		break;
	case DXGI_MODE_ROTATION_ROTATE270:
	case DXGI_MODE_ROTATION_ROTATE90:
		for (int srcY = 0; srcY < (int)m_latestBmp.Height; srcY++) {
			uint32_t* srcRow = src + srcY * m_latestBmp.Width;
			int dstX = finalWidth - 1 - srcY;
			for (int srcX = 0; srcX < (int)m_latestBmp.Width; srcX++) {
				dst[srcX * finalWidth + dstX] = srcRow[srcX];
			}
		}
		break;
	}

	ReleaseDC(NULL, hdc);
	return dib;
}