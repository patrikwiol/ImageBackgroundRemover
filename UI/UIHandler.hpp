#pragma once

namespace UIHandler
{
	inline ID3D11Device* device = nullptr;
	inline ID3D11DeviceContext* deviceContext = nullptr;
	inline IDXGISwapChain* swapChain = nullptr;
	inline bool swapChainOccluded = false;
	inline UINT resizeWidth = 0, resizeHeight = 0;
	inline ID3D11RenderTargetView* mainRenderTargetView = nullptr;
	inline HWND window{};
	inline WNDCLASSEX wc{};

	int Init(HINSTANCE inst);
	void Cleanup();

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	uint8_t SetupMain();
	void FinishMain(ImGuiIO& io);

	void MainPage();

	void RenderMain(ImGuiIO& io);
}