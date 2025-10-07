#include "WinDesktopDup.h"

WinDesktopDup::~WinDesktopDup() {
	Close();
}

BOOL WinDesktopDup::Initialize() {
	// Get desktop
	HDESK hDesk = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hDesk)
		return FALSE;

	// Attach desktop to this thread (presumably for cases where this is not the main/UI thread)
	bool deskAttached = SetThreadDesktop(hDesk) != 0;
	CloseDesktop(hDesk);
	hDesk = nullptr;
	if (!deskAttached) {
		DWORD error = GetLastError();
		if (error != 170)
			return FALSE;
	}

	// Initialize DirectX
	HRESULT hr = S_OK;

	// Driver types supported
	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	auto numDriverTypes = ARRAYSIZE(driverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1 };
	auto numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel;

	// Create device
	for (size_t i = 0; i < numDriverTypes; i++) {
		hr = D3D11CreateDevice(nullptr, driverTypes[i], nullptr, 0, featureLevels, (UINT)numFeatureLevels,
			D3D11_SDK_VERSION, &D3DDevice, &featureLevel, &D3DDeviceContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return FALSE;

	// Initialize the Desktop Duplication system
	//m_OutputNumber = Output;

	// Get DXGI device
	IDXGIDevice* dxgiDevice = nullptr;
	hr = D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr))
		return FALSE;

	// Get DXGI adapter
	IDXGIAdapter* dxgiAdapter = nullptr;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	dxgiDevice->Release();
	dxgiDevice = nullptr;
	if (FAILED(hr))
		return FALSE;

	// Get outputs
	std::vector<IDXGIOutput*> outputs;
	IDXGIOutput* dxgiOutput = nullptr;
	for (UINT i = 0; dxgiAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; ++i) {
		if (dxgiOutput) {
			outputs.push_back(dxgiOutput);
		}
	}

	dxgiAdapter->Release();
	dxgiAdapter = nullptr;
	if (FAILED(hr))
		return FALSE;

	DXGI_OUTPUT_DESC OutputDesc;
	IDXGIOutputDuplication* DeskDupl;

	for (size_t i = 0; i < outputs.size(); i++)
	{
		dxgiOutput = outputs[i];
		dxgiOutput->GetDesc(&OutputDesc);

		OutputDescs.push_back(OutputDesc);

		IDXGIOutput1* dxgiOutput1 = nullptr;
		hr = dxgiOutput->QueryInterface(__uuidof(dxgiOutput1), (void**)&dxgiOutput1);
		dxgiOutput->Release();
		dxgiOutput = nullptr;
		if (FAILED(hr))
			return FALSE;

		hr = dxgiOutput1->DuplicateOutput(D3DDevice, &DeskDupl);
		dxgiOutput1->Release();
		dxgiOutput1 = nullptr;
		if (FAILED(hr))
			return FALSE;

		DeskDupls.push_back(DeskDupl);
		HaveFrameLocks.push_back(false);
	}

	Enabled = true;
	return TRUE;
}

void WinDesktopDup::Close() {
	for (size_t i = 0; i < DeskDupls.size(); i++)
	{
		IDXGIOutputDuplication* DeskDupl = DeskDupls[i];
		if (DeskDupl)
			DeskDupl->Release();
	}

	if (D3DDeviceContext)
		D3DDeviceContext->Release();

	if (D3DDevice)
		D3DDevice->Release();

	DeskDupls.clear();
	HaveFrameLocks.clear();
	OutputDescs.clear();
	D3DDeviceContext = nullptr;
	D3DDevice = nullptr;
	Enabled = false;
}

void WinDesktopDup::Reinitialize()
{
	Close();
	Initialize();
}

HBITMAP WinDesktopDup::CaptureNext(int index) {
	if (DeskDupls.size() == 0)
		return NULL;

	if (index == -1)
	{
		int leftMostX = 0, rightMostX = 0;
		int topMostY = 0, bottomMostY = 0;

		for (size_t i = 0; i < OutputDescs.size(); i++)
		{
			if (OutputDescs[i].DesktopCoordinates.left < leftMostX)
				leftMostX = OutputDescs[i].DesktopCoordinates.left;

			if (OutputDescs[i].DesktopCoordinates.right > rightMostX)
				rightMostX = OutputDescs[i].DesktopCoordinates.right;

			if (OutputDescs[i].DesktopCoordinates.bottom > bottomMostY)
				bottomMostY = OutputDescs[i].DesktopCoordinates.bottom;

			if (OutputDescs[i].DesktopCoordinates.top < topMostY)
				topMostY = OutputDescs[i].DesktopCoordinates.top;
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
		void* bits = nullptr;
		HBITMAP dib = CreateDIBSection(hdc, &inf, 0, &bits, nullptr, 0);

		if (!dib) return FALSE;

		HDC hdcDest = CreateCompatibleDC(hdc);
		HDC hdcSrc = CreateCompatibleDC(hdc);
		HBITMAP oldDest = (HBITMAP)SelectObject(hdcDest, dib);

		for (size_t i = 0; i < DeskDupls.size(); i++)
		{
			int xCord = OutputDescs[i].DesktopCoordinates.left - leftMostX;
			int yCord = OutputDescs[i].DesktopCoordinates.top - topMostY;

			int bmpWidth = OutputDescs[i].DesktopCoordinates.right - OutputDescs[i].DesktopCoordinates.left;
			int bmpHeight = OutputDescs[i].DesktopCoordinates.bottom - OutputDescs[i].DesktopCoordinates.top;

			HBITMAP bmp = CaptureNext(i);

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
		if ((size_t)index >= DeskDupls.size())
			return NULL;

		HRESULT hr;
		IDXGIOutputDuplication* DeskDupl = DeskDupls[index];

		if (HaveFrameLocks[index]) {
			HaveFrameLocks[index] = false;
			hr = DeskDupl->ReleaseFrame();
		}

		IDXGIResource* deskRes = nullptr;
		DXGI_OUTDUPL_FRAME_INFO frameInfo;

		hr = DeskDupl->AcquireNextFrame(100000, &frameInfo, &deskRes);
		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			return NULL;

		if (FAILED(hr)) {
			Reinitialize();
			return NULL;
		}

		HaveFrameLocks[index] = true;

		ID3D11Texture2D* gpuTex = nullptr;
		hr = deskRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&gpuTex);
		deskRes->Release();
		deskRes = nullptr;
		if (FAILED(hr))
			return NULL;

		bool ok = true;

		D3D11_TEXTURE2D_DESC desc;
		gpuTex->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		ID3D11Texture2D* cpuTex = nullptr;
		hr = D3DDevice->CreateTexture2D(&desc, nullptr, &cpuTex);

		if (SUCCEEDED(hr))
			D3DDeviceContext->CopyResource(cpuTex, gpuTex);
		else
			ok = false;

		if (!cpuTex) return FALSE;

		D3D11_MAPPED_SUBRESOURCE sr;
		hr = D3DDeviceContext->Map(cpuTex, 0, D3D11_MAP_READ, 0, &sr);
		if (SUCCEEDED(hr)) {
			if (Latest.Width != desc.Width || Latest.Height != desc.Height) {
				Latest.Width = desc.Width;
				Latest.Height = desc.Height;
				Latest.Buf.resize(desc.Width * desc.Height * 4);
			}
			for (int y = 0; y < (int)desc.Height; y++)
				memcpy(Latest.Buf.data() + y * desc.Width * 4, (uint8_t*)sr.pData + sr.RowPitch * y, desc.Width * 4);
			D3DDeviceContext->Unmap(cpuTex, 0);
		}
		else {
			ok = false;
		}

		cpuTex->Release();
		gpuTex->Release();

		if (!ok)
			return NULL;

		HBITMAP res = GetHBITMAP(index);
		Latest.Buf.clear();

		return res;
	}
}

HBITMAP WinDesktopDup::GetHBITMAP(int index) {
	HDC hdc = GetDC(NULL);
	BITMAPINFO inf;
	memset(&inf, 0, sizeof(inf));

	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
	if (index >= 0 && index < (int)OutputDescs.size()) {
		rotation = OutputDescs[index].Rotation;
	}

	int finalWidth = Latest.Width;
	int finalHeight = Latest.Height;

	if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
		finalWidth = Latest.Height;
		finalHeight = Latest.Width;
	}

	inf.bmiHeader.biSize = sizeof(inf.bmiHeader);
	inf.bmiHeader.biWidth = finalWidth;
	inf.bmiHeader.biHeight = -finalHeight;
	inf.bmiHeader.biPlanes = 1;
	inf.bmiHeader.biBitCount = 32;
	inf.bmiHeader.biCompression = BI_RGB;

	void* bits = nullptr;
	HBITMAP dib = CreateDIBSection(hdc, &inf, 0, &bits, nullptr, 0);

	uint32_t* src = (uint32_t*)Latest.Buf.data();
	uint32_t* dst = (uint32_t*)bits;

	switch (rotation) {
	case DXGI_MODE_ROTATION_IDENTITY:
	case DXGI_MODE_ROTATION_UNSPECIFIED:
	case DXGI_MODE_ROTATION_ROTATE180:
		memcpy(bits, Latest.Buf.data(), Latest.Width * Latest.Height * 4);
		break;
	case DXGI_MODE_ROTATION_ROTATE270:
	case DXGI_MODE_ROTATION_ROTATE90:
		for (int srcY = 0; srcY < (int)Latest.Height; srcY++) {
			uint32_t* srcRow = src + srcY * Latest.Width;
			int dstX = finalWidth - 1 - srcY;
			for (int srcX = 0; srcX < (int)Latest.Width; srcX++) {
				dst[srcX * finalWidth + dstX] = srcRow[srcX];
			}
		}
		break;
	}

	ReleaseDC(NULL, hdc);
	return dib;
}