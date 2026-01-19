#include <precompiled.h>
#include "UI/UIHandler.hpp"

int WinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE prevInst, _In_ LPSTR cmdLine, _In_ int cmdShow)
{
	if (UIHandler::Init(inst))
		return 1;

	ImGuiIO& io = ImGui::GetIO();

	uint8_t retryTicks = 0;
	while (true)
	{
		if (UI::data.shouldExit)
			break;

		const uint8_t result = UIHandler::SetupMain();
		if (result == 1 || retryTicks >= 10)
			break;

		if (result == 2)
		{
			Sleep(50);
			++retryTicks;
			continue;
		}

		UIHandler::RenderMain(io);
		UIHandler::FinishMain(io);
	}

	UIHandler::Cleanup();
	return 0;
}