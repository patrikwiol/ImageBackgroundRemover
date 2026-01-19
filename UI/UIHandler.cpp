#include <precompiled.h>
#include "UIHandler.hpp"
#include "../Features/Utils.hpp"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int UIHandler::Init(HINSTANCE inst)
{
	ImGui_ImplWin32_EnableDpiAwareness();

	std::memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = (L"ImageBackgroundRemover");
	RegisterClassExW(&wc);

	window = CreateWindowEx(
		WS_EX_APPWINDOW,
		wc.lpszClassName,
		L"ImageBackgroundRemover",
		WS_POPUP | WS_VISIBLE,
		0, 0, 1, 1,
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);
	if (!UIHandler::CreateDeviceD3D(window))
	{
		UIHandler::CleanupDeviceD3D();
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	UI::CustomStyle();

	ImGuiStyle& style = ImGui::GetStyle();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, deviceContext);

	UI::LoadFonts(io);

	RECT screenRect{};
	GetWindowRect(GetDesktopWindow(), &screenRect);
	UI::data.screenSize = ImVec2(float(screenRect.right), float(screenRect.bottom));
	UI::data.windowPos = ImVec2((UI::data.screenSize.x - UI::data.windowSize.x) * 0.5f, (UI::data.screenSize.y - UI::data.windowSize.y) * 0.5f);
	return 0;
}

void UIHandler::Cleanup()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	UIHandler::CleanupDeviceD3D();
	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool UIHandler::CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &deviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &deviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void UIHandler::CleanupDeviceD3D()
{
	CleanupRenderTarget();

	if (swapChain)
	{
		swapChain->Release();
		swapChain = nullptr;
	}

	if (deviceContext)
	{
		deviceContext->Release();
		deviceContext = nullptr;
	}

	if (device)
	{
		device->Release();
		device = nullptr;
	}
}

void UIHandler::CreateRenderTarget()
{
	ID3D11Texture2D* backBuffer;
	swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (!backBuffer)
		return;

	device->CreateRenderTargetView(backBuffer, nullptr, &mainRenderTargetView);
	backBuffer->Release();
}

void UIHandler::CleanupRenderTarget()
{
	if (mainRenderTargetView)
	{
		mainRenderTargetView->Release();
		mainRenderTargetView = nullptr;
	}
}

uint8_t UIHandler::SetupMain()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return 1;
	}

	if (swapChainOccluded && swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
		return 2;
	swapChainOccluded = false;

	if (resizeWidth != 0 && resizeHeight != 0)
	{
		CleanupRenderTarget();
		swapChain->ResizeBuffers(0, resizeWidth, resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		resizeWidth = resizeHeight = 0;
		CreateRenderTarget();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return 0;
}

void UIHandler::FinishMain(ImGuiIO& io)
{
	ImGui::EndFrame();
	ImGui::Render();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	deviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
	deviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	HRESULT hr = swapChain->Present(0, 0);
	swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		UIHandler::resizeWidth = (UINT)LOWORD(lParam);
		UIHandler::resizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void UIHandler::RenderMain(ImGuiIO& io)
{
	ImGui::PushFont(UI::fonts.mainFont.normalDefault);
	ImGui::SetNextWindowPos(ImVec2(UI::data.windowPos.x, UI::data.windowPos.y), ImGuiCond_Once);

	static int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin("Image Background Remover", 0, flags);
	{
		ImGui::SetWindowSize(ImVec2{ UI::data.windowSize.x * UI::state.menuMod, UI::data.windowSize.y * UI::state.menuMod });
		const auto window = ImGui::GetCurrentWindow();
		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		const ImVec2 factor((UI::data.windowSize.x * UI::state.menuMod), (UI::data.windowSize.y * UI::state.menuMod));
		const ImVec2 pos(window->WorkRect.GetCenter().x, window->WorkRect.GetCenter().y);

		if (UI::data.stage != UI::STAGE_STARTING)
			UI::Header("Image Background Remover");

		switch (UI::data.stage)
		{
		case UI::STAGE_STARTING:
		{
			UI::state.menuMod = ImLerp(UI::state.menuMod, 1.f, io.DeltaTime * UI::animSpeed);

			ImGui::SetWindowPos(ImVec2(UI::data.windowPos.x - factor.x * 0.5f + UI::data.windowSize.x * 0.5f, UI::data.windowPos.y - factor.y * 0.5f + UI::data.windowSize.y * 0.5f));
			ImGui::SetWindowSize(factor);

			if (UI::state.menuMod >= 0.999f)
			{
				UI::state.menuMod = 1.f;
				UI::data.stage = UI::STAGE_LOADING;
			}

			break;
		}
		case UI::STAGE_LOADING:
		{
			UI::LoadingCircle("Loading", UI::loadingRadius, 4, ImGui::GetColorU32(UI::primaryCol), pos);
			UI::data.stage = UI::STAGE_MAIN;
			break;
		}
		case UI::STAGE_MAIN:
		{
			MainPage();
			break;
		}
		}
	}
	ImGui::End();

	ImGui::PopFont();
}

/* TODO: clean this mess up */
void UIHandler::MainPage()
{
	const auto window = ImGui::GetCurrentWindow();
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const auto draw = window->DrawList;
	const ImGuiID id = window->GetID("Main Page");
	const ImGuiID inputID = window->GetID("Input Image");
	const ImGuiID outputID = window->GetID("Output Image");

	const ImVec2 rectSize(UI::data.windowSize / 2.1f);
	const ImVec2 pos(window->WorkRect.Min.x, window->WorkRect.Min.y + 45.f);
	const ImVec2 endPos(window->WorkRect.Max.x, window->WorkRect.Max.y);

	const ImRect inputImgBB(ImVec2(pos.x + style.WindowPadding.x, pos.y + style.WindowPadding.y), ImVec2(pos.x + style.WindowPadding.x + rectSize.x, pos.y + style.WindowPadding.y + rectSize.y));
	const ImRect outputImgBB(ImVec2(endPos.x - style.WindowPadding.x - rectSize.x, pos.y + style.WindowPadding.y), ImVec2(endPos.x - style.WindowPadding.x, pos.y + style.WindowPadding.y + rectSize.y));

	const bool inputHovered = ImGui::IsMouseHoveringRect(inputImgBB.Min, inputImgBB.Max);
	const bool inputClicked = inputHovered && g.IO.MouseClicked[0];

	const bool outputHovered = ImGui::IsMouseHoveringRect(outputImgBB.Min, outputImgBB.Max);
	const bool outputClicked = outputHovered && g.IO.MouseClicked[0];

	static cv::Mat inputImg{};
	static cv::Mat processedImg{};
	static cv::Mat processedImgRGBA{};
	static ID3D11ShaderResourceView* inputTexture = nullptr;
	static ID3D11ShaderResourceView* outputTexture = nullptr;

	static bool isProcessing = false;
	static bool isProcessingInput = false;
	static bool needsTextureUpdate = false;
	if (inputClicked && !isProcessing)
	{
		ImGui::SetActiveID(inputID, window);
		const std::wstring path = Utils::OpenFileExplorer(false, Utils::FT_IMAGE);

		if (!path.empty())
		{
			isProcessing = true;
			isProcessingInput = true;

			Utils::Options options = Utils::options;

			std::thread([path, options]() {
				const std::string sPath(path.begin(), path.end());
				cv::Mat rawImg = cv::imread(sPath);

				if (!rawImg.empty())
				{
					cv::cvtColor(rawImg, inputImg, cv::COLOR_BGR2RGBA);
					needsTextureUpdate = true;
					isProcessingInput = false;

					processedImg = Utils::RemoveBackground(rawImg, options);
					cv::cvtColor(processedImg, processedImgRGBA, cv::COLOR_BGR2RGBA);
					needsTextureUpdate = true;
				}
				isProcessing = false;
				}).detach();
		}
	}
	else if (outputClicked)
	{
		ImGui::SetActiveID(outputID, window);
		const std::wstring path = Utils::OpenFileExplorer(true);
		
		if (!path.empty() && !processedImg.empty())
		{
			/* TODO: let user decide what to name the img */
			const std::wstring finalSavePath = path + L"\\imgResult.png";

			const std::string sPath(finalSavePath.begin(), finalSavePath.end());
			cv::imwrite(sPath, processedImg);
		}
	}

	if (needsTextureUpdate)
	{
		if (inputTexture)
			inputTexture->Release();
		if (outputTexture)
			outputTexture->Release();

		if (!inputImg.empty())
			inputTexture = Utils::MatToTexture(device, inputImg);

		if (!processedImgRGBA.empty())
			outputTexture = Utils::MatToTexture(device, processedImgRGBA);

		needsTextureUpdate = false;
	}

	UI::AnimState& state = UI::anim[id];

	constexpr ImVec4 hoverCol(1.f, 1.f, 1.f, 0.1f);
	constexpr ImVec4 normalCol(1.f, 1.f, 1.f, 0.f);

	state.filled = ImLerp(state.filled, inputHovered ? hoverCol : normalCol, UI::animSpeed * g.IO.DeltaTime);
	state.filled2 = ImLerp(state.filled2, outputHovered ? hoverCol : normalCol, UI::animSpeed * g.IO.DeltaTime);

	ImGui::PushFont(UI::fonts.mainFont.bigDefault);
	const char* inputText = "Input Image (Click to select Image)";
	const ImVec2 inputTextSize = ImGui::CalcTextSize(inputText);
	draw->AddText(ImVec2(inputImgBB.GetCenter().x - inputTextSize.x / 2.f, inputImgBB.Min.y - inputTextSize.y - 5), ImGui::GetColorU32(UI::textCol), inputText);

	if (inputTexture)
		draw->AddImageRounded(inputTexture, inputImgBB.Min, inputImgBB.Max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, UI::windowRounding);
	
	draw->AddRectFilled(inputImgBB.Min, inputImgBB.Max, ImGui::GetColorU32(state.filled), UI::windowRounding);
	draw->AddRect(inputImgBB.Min, inputImgBB.Max, ImGui::GetColorU32(UI::bgColDark4), UI::windowRounding, 0, 2.f);
	
	if (isProcessingInput)
		UI::LoadingCircle("Loading Input Image", UI::loadingRadius, 4, ImGui::GetColorU32(UI::primaryCol), inputImgBB.GetCenter());
	
	if (!inputTexture && !isProcessingInput)
	{
		ImGui::PushFont(UI::fonts.iconFont.hugeBold);
		const char* iconText = "c";
		const ImVec2 iconTextSize = ImGui::CalcTextSize(iconText);
		draw->AddText(ImVec2(inputImgBB.GetCenter() - iconTextSize / 2), ImGui::GetColorU32(UI::textCol), "c");
		ImGui::PopFont();
	}

	const char* outputText = "Output Image (Click to save Image)";
	const ImVec2 outputTextSize = ImGui::CalcTextSize(outputText);
	draw->AddText(ImVec2(outputImgBB.GetCenter().x - outputTextSize.x / 2.f, outputImgBB.Min.y - outputTextSize.y - 5), ImGui::GetColorU32(UI::textCol), outputText);

	if (outputTexture)
		draw->AddImageRounded(outputTexture, outputImgBB.Min, outputImgBB.Max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, UI::windowRounding);
	
	draw->AddRectFilled(outputImgBB.Min, outputImgBB.Max, ImGui::GetColorU32(state.filled2), UI::windowRounding);
	draw->AddRect(outputImgBB.Min, outputImgBB.Max, ImGui::GetColorU32(UI::bgColDark4), UI::windowRounding, 0, 2.f);
	
	if (isProcessing)
		UI::LoadingCircle("Loading Output Image", UI::loadingRadius, 4, ImGui::GetColorU32(UI::primaryCol), outputImgBB.GetCenter());
	
	if (outputTexture)
	{
		const std::string timeTakenText = "Took " + std::to_string(Utils::elapsedTime) + "s to remove background";
		const ImVec2 timeTakenTextSize = ImGui::CalcTextSize(timeTakenText.c_str());
		draw->AddText(ImVec2(window->WorkRect.GetCenter().x - timeTakenTextSize.x / 2.f, inputImgBB.Max.y + 10), ImGui::GetColorU32(UI::textCol), timeTakenText.c_str());
	}

	/* TODO: do this dynamically, too lazy atm */
	ImGui::SetCursorPosY(520.f);
	UI::BeginChild("Options", ImVec2(window->WorkRect.GetSize().x, 350.f), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_Borders);
	{
		UI::Checkbox("Remove Specific Color", &Utils::options.removeSpecificColor, Utils::options.specificColor);
		if (!Utils::options.removeSpecificColor)
		{
			UI::SliderInt("Iterations", &Utils::options.iterations, 1, 25);
			UI::ToolTip("Higher = More accurate, but slower");

			UI::Checkbox("Downscale Image for background removal", &Utils::options.downscaleImage);
			UI::ToolTip("Usually no noticable difference in accuracy\nImage gets upscaled again afterwards");
		}
		else
		{
			UI::SliderFloat("Accuracy", &Utils::options.accuracy, 0.f, 1.f, "%.2f");
			UI::ToolTip("Lower values remove a wider range of similar colors");
		}

		UI::Checkbox("Remove background outline", &Utils::options.removeOutline);
		UI::ToolTip("Sometimes an outline of the background will form around the foreground\nThis aims to get rid of that");

		if (Utils::options.removeOutline)
		{
			UI::SliderInt("Erosion Size", &Utils::options.erosionSize, 1, 5);
			UI::ToolTip("Shrinks the mask to remove outlines\nUse 1 or 2 to clean up messy edges");
		}
	}
	UI::EndChild();

	ImGui::PopFont();
}