#include <precompiled.h>
#include "UI.hpp"
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "../UI/Misc/Textures/stb_image.h"

static ImColor d = ImColor(0, 0, 0, 0);

static void ResetAnimData()
{
	UI::anim.clear();
}

void MinimizeApp()
{
	/* TODO */
}

static void ExitApp()
{
	UI::data.shouldExit = true;
}

void UI::Header(const char* text)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const auto draw = window->DrawList;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImVec2 pos = window->DC.CursorPos;
	const ImRect workRect = window->WorkRect;

	ImGui::PushFont(fonts.mainFont.bigDefault);

	const float textHeight = ImGui::CalcTextSize(text).y;

	const ImRect totalBB(ImVec2(workRect.Min.x - style.WindowPadding.x, workRect.Min.y - style.WindowPadding.y), ImVec2(workRect.Max.x + style.WindowPadding.x, workRect.Min.y + textHeight + 5));
	const ImRect bb(workRect.Min, ImVec2(workRect.Max.x, workRect.Min.y + textHeight));

	draw->AddRectFilled(totalBB.Min, totalBB.Max, ImGui::GetColorU32(bgColDark2), windowRounding + 3, ImDrawFlags_RoundCornersTop);

	draw->AddText(ImVec2(bb.Min.x, totalBB.GetCenter().y - textHeight / 2), ImGui::GetColorU32(textCol), text);

	auto DrawClickableCircle = [&](ImVec2 pos, const char* text, float radius, ImVec4 col, auto onClickAction)
	{
		const ImGuiID id = window->GetID(text);

		const ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);

		const bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + radius * 2, pos.y + radius * 2));
		const bool clicked = hovered && g.IO.MouseClicked[0];

		if (clicked)
			ImGui::SetActiveID(id, window);

		AnimState& state = UI::anim[id];
		state.filled = ImLerp(state.filled, hovered ? col : transCol, g.IO.DeltaTime * animSpeed);
		
		draw->AddCircleFilled(center, radius, ImGui::GetColorU32(state.filled));
		const ImVec2 textSize = ImGui::CalcTextSize(text);

		draw->AddText(ImVec2(center.x - textSize.x / 2, center.y - textSize.y / 2), ImGui::GetColorU32(textCol), text);

		if (hovered && clicked)
			onClickAction();
	};

	const float radius = 10;

	ImGui::PushFont(fonts.iconFont.normalDefault);

	DrawClickableCircle(ImVec2(bb.Max.x - radius * 2, bb.GetCenter().y - radius), "a", radius, redCol, []() { ExitApp(); });
	//DrawClickableCircle(ImVec2(bb.Max.x - (radius * 4 + 5), bb.GetCenter().y - radius), "b", radius, bgColLight3, []() { MinimizeApp(); });

	ImGui::PopFont();

	ImGui::PopFont();
}

void UI::BeginChild(const char* label, const ImVec2 childSize, ImGuiChildFlags flags)
{
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const float offset = childSize.x;

	std::string sLabel = std::string(label) + (" Wrapper");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::BeginChild(sLabel.c_str(), childSize, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | flags, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGui::PushFont(fonts.size == 20 ? fonts.mainFont.largeBold : fonts.size == 12 ? fonts.mainFont.normalBold : fonts.mainFont.bigBold);

	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 textSize = ImGui::CalcTextSize(label);

	const ImVec2 start = ImVec2(pos.x, pos.y);
	const ImVec2 end = ImVec2(pos.x + childSize.x, pos.y + textSize.y + style.FramePadding.y);
	const ImRect bb = ImRect(start, end);

	ImGui::PushStyleColor(ImGuiCol_Text, textCol);
	window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(bgColLight), style.ChildRounding, ImDrawFlags_RoundCornersTop);
	ImGui::RenderText(ImVec2(pos.x + style.WindowPadding.x, bb.GetCenter().y - textSize.y / 2), label);
	ImGui::PopStyleColor();

	ImGui::PopFont();

	const float secondChildHeight = childSize.y - bb.GetHeight();

	ImGui::SetCursorPos(ImVec2(0.f, bb.GetHeight()));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::BeginChild(label, ImVec2(childSize.x, secondChildHeight), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_Borders);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}

void UI::EndChild()
{
	ImGui::EndChild();
	ImGui::EndChild();
}

static bool BaseCheckbox(const char* label, bool* v, ImColor& col, ImColor& col2, int colorAmt)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	UI::window = window;
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);
	const float squareSz = ImGui::GetFrameHeight();
	const ImVec2 pos = window->DC.CursorPos;

	ImVec2 end(window->WorkRect.Max.x, pos.y + labelSize.y + style.FramePadding.y * 2.0f);
	const ImRect totalBB(pos, end);

	ImVec2 start(end.x - squareSz, pos.y);
	const ImRect checkBB(start, end);

	ImGui::ItemSize(totalBB, style.FramePadding.y);

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(checkBB, id, &hovered, &held);
	bool checked = *v;

	if (pressed)
		checked = !checked;

	if (*v != checked)
	{
		*v = checked;
		pressed = true;
		ImGui::MarkItemEdited(id);
	}

	UI::AnimState& state = UI::anim[id];

	state.filled = ImLerp(state.filled, *v ? UI::primaryCol : hovered ? UI::bgColLight : UI::bgColDark, g.IO.DeltaTime * UI::animSpeed);
	state.text = ImLerp(state.text, hovered ? UI::textCol2 : UI::textCol, g.IO.DeltaTime * UI::animSpeed);
	state.checkMark = ImLerp(state.checkMark, *v ? UI::textCol2 : hovered ? UI::transCol : UI::transCol, g.IO.DeltaTime * UI::animSpeed);
	state.alpha = ImLerp(state.alpha, *v ? 1.f : 0.f, g.IO.DeltaTime * UI::animSpeed);

	ImGui::PushStyleColor(ImGuiCol_Text, state.text);
	ImGui::RenderText(ImVec2(pos.x, checkBB.GetCenter().y - labelSize.y / 2), label);
	ImGui::PopStyleColor();

	ImGui::RenderNavCursor(totalBB, id);
	ImGui::RenderFrame(checkBB.Min, checkBB.Max, ImGui::GetColorU32(state.filled), false, style.FrameRounding);

	if (state.alpha > 0.1f)
	{
		const float pad = ImMax(1.f, IM_TRUNC(squareSz / 5.f));
		ImGui::RenderCheckMark(window->DrawList, ImVec2(checkBB.Min.x + pad, checkBB.Min.y + pad), ImGui::GetColorU32(state.checkMark), squareSz - pad * 2.f, state.alpha);
	}

	std::string newLabel = std::string(label) + (" Color");

	switch (colorAmt)
	{
	case 0:
		break;
	case 1:
		ImGui::SameLine();
		UI::ColorPicker(newLabel.c_str(), col, 1);
		break;
	case 2:
		ImGui::SameLine();
		UI::ColorPicker(newLabel.c_str(), col, 1);

		newLabel += ("2");
		ImGui::SameLine();
		UI::ColorPicker(newLabel.c_str(), col2, 2);
		break;
	}

	ImGui::ItemAdd(totalBB, id);

	return pressed;
}

bool UI::Checkbox(const char* label, bool* v)
{
	return BaseCheckbox(label, v, d, d, 0);
}

bool UI::Checkbox(const char* label, bool* v, ImColor& col)
{
	return BaseCheckbox(label, v, col, d, 1);
}

bool UI::Checkbox(const char* label, bool* v, ImColor& col, ImColor& col2)
{
	return BaseCheckbox(label, v, col, col2, 2);
}

static bool BaseSlider(const char* label, void* v, const void* vMin, const void* vMax, const char* format, ImGuiDataType dataType, const char* addon)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	UI::window = window;
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);
	const float squareSz = ImGui::GetFrameHeight();

	ImVec2 end(window->WorkRect.Max.x, pos.y + labelSize.y + style.FramePadding.y * 2.0f);
	const ImRect bb(pos, end);

	ImVec2 start(end.x - squareSz * 8 - style.ItemInnerSpacing.x, pos.y);
	ImVec2 end2(end.x - squareSz * 6 - style.ItemInnerSpacing.x, end.y);
	const ImRect valueBB(start, end2);

	ImVec2 start2(end.x - squareSz * 6, pos.y);
	const ImRect sliderBB(start2, end);

	ImGui::ItemSize(bb, style.FramePadding.y);
	ImGui::ItemAdd(bb, id, &valueBB, ImGuiItemFlags_Inputable);

	if (format == NULL)
		format = ImGui::DataTypeGetInfo(dataType)->PrintFmt;

	const bool hovered = ImGui::ItemHoverable(sliderBB, id, g.LastItemData.ItemFlags);
	const bool clicked = (hovered && g.IO.MouseClicked[0]);
	bool tempInput = ImGui::TempInputIsActive(id);
	const bool vHovered = ImGui::IsMouseHoveringRect(valueBB.Min, valueBB.Max);
	const bool vClicked = vHovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);

	if (!tempInput)
	{
		const bool makeActive = (vClicked || g.NavActivateId == id);
		if (makeActive && vClicked)
			ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
		if (makeActive)
			if ((vClicked) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
				tempInput = true;

		if (makeActive)
			memcpy(&g.ActiveIdValueOnActivation, v, ImGui::DataTypeGetInfo(dataType)->Size);

		if (makeActive && !tempInput)
		{
			ImGui::SetActiveID(id, window);
			ImGui::SetFocusID(id, window);
			ImGui::FocusWindow(window);
			g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
		}
	}

	if (clicked && !tempInput)
	{
		ImGui::SetActiveID(id, window);
		ImGui::SetFocusID(id, window);
		ImGui::FocusWindow(window);
		g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
	}

	UI::AnimState& state = UI::anim[id];

	state.filled2 = ImLerp(state.filled2, vClicked ? UI::bgColLight : vHovered ? UI::bgColLight : UI::bgColDark, g.IO.DeltaTime * UI::animSpeed);

	ImGui::RenderFrame(ImVec2(valueBB.Min.x + 1, valueBB.Min.y + 1), ImVec2(valueBB.Max.x - 1, valueBB.Max.y - 1), ImGui::GetColorU32(state.filled2), false, style.FrameRounding);

	bool valueChanged = false;

	ImRect grabBB{};

	if (tempInput)
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, UI::bgColDark);
		valueChanged = ImGui::TempInputScalar(valueBB, id, label, dataType, v, format, vMin, vMax);
		ImGui::PopStyleColor();
	}
	else
	{
		valueChanged = ImGui::SliderBehavior(sliderBB, id, dataType, v, vMin, vMax, format, NULL, &grabBB);

		state.pos = grabBB.Min;
		state.pos2 = grabBB.Max;
	}

	grabBB.Min = state.pos;
	grabBB.Max = state.pos2;

	if (valueChanged)
		ImGui::MarkItemEdited(id);

	char valueBuf[64];
	char valueMin[64];
	char valueMax[64];
	ImGui::DataTypeFormatString(valueBuf, IM_ARRAYSIZE(valueBuf), dataType, v, format);
	ImGui::DataTypeFormatString(valueMin, IM_ARRAYSIZE(valueMin), dataType, vMin, format);
	ImGui::DataTypeFormatString(valueMax, IM_ARRAYSIZE(valueMax), dataType, vMax, format);

	const float percent = (grabBB.Max.x - sliderBB.Min.x);

	state.sliderPercent = ImLerp(state.sliderPercent, percent, g.IO.DeltaTime * (UI::animSpeed * 1.2f));
	state.filled = ImLerp(state.filled, clicked ? UI::bgColLight : hovered ? UI::bgColLight : UI::bgColDark, g.IO.DeltaTime * UI::animSpeed);
	state.text = ImLerp(state.text, hovered ? UI::textCol2 : UI::textCol, g.IO.DeltaTime * UI::animSpeed);
	state.text2 = ImLerp(state.text2, vHovered ? UI::textCol2 : UI::textCol, g.IO.DeltaTime * UI::animSpeed);

	ImGui::PushStyleColor(ImGuiCol_Text, state.text);
	ImGui::RenderText(ImVec2(pos.x, sliderBB.GetCenter().y - ImGui::CalcTextSize(label).y / 2), label);
	ImGui::PopStyleColor();

	ImGui::RenderFrame(ImVec2(sliderBB.Min.x + 1, sliderBB.Min.y + 1), ImVec2(sliderBB.Max.x - 1, sliderBB.Max.y - 1), ImGui::GetColorU32(state.filled), false, style.FrameRounding);
	ImGui::RenderFrame(ImVec2(sliderBB.Min.x + 3, sliderBB.Min.y + 4), ImVec2(sliderBB.Min.x + state.sliderPercent, sliderBB.Max.y - 4), ImGui::GetColorU32(UI::primaryCol), false, style.FrameRounding - 1);

	if (!tempInput)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, state.text2);
		std::string sValue = valueBuf;
		sValue += addon;

		const ImVec2 valueSize = ImGui::CalcTextSize(sValue.c_str());
		ImGui::RenderText(ImVec2(valueBB.GetCenter().x - valueSize.x / 2, valueBB.GetCenter().y - valueSize.y / 2), sValue.c_str());
		ImGui::PopStyleColor();
	}

	return valueChanged;
}

bool UI::SliderInt(const char* label, int* v, int vMin, int vMax, const char* format, const char* addon)
{
	return BaseSlider(label, v, &vMin, &vMax, format, ImGuiDataType_S32, addon);
}

bool UI::SliderFloat(const char* label, float* v, float vMin, float vMax, const char* format)
{
	return BaseSlider(label, v, &vMin, &vMax, format, ImGuiDataType_Float, "");
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
	ImGuiContext& g = *GImGui;
	if (items_count <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

static void RenderArrow(ImDrawList* drawList, ImVec2 pos, float scale, bool enabled, float& factor, ImU32 col)
{
	ImGuiContext& g = *GImGui;
	const auto arrowSize = ImTrunc(g.FontSize * scale + g.Style.FramePadding.x);
	const ImVec2 arrowPos(pos.x - arrowSize / 2, pos.y - arrowSize / 2);

	const float h = drawList->_Data->FontSize * 1.f;
	float r = h * 0.4f * 1.f;
	const ImVec2 center = ImVec2(arrowPos.x + h * 0.5f, arrowPos.y + h * 0.5f * 1.f);

	ImVec2 a, b, c;
	if (enabled)
		r = -r;

	factor = ImLerp(factor, r, g.IO.DeltaTime * UI::animSpeed);

	a = ImVec2(+0.000f * factor, +0.750f * factor);
	b = ImVec2(-0.866f * factor, -0.750f * factor);
	c = ImVec2(+0.866f * factor, -0.750f * factor);

	const ImVec2 centeredA(center.x + a.x, center.y + a.y);
	const ImVec2 centeredB(center.x + b.x, center.y + b.y);
	const ImVec2 centeredC(center.x + c.x, center.y + c.y);

	drawList->AddTriangleFilled(centeredA, centeredB, centeredC, col);
}

static bool BaseCombo(const char* label, const char* preview, int itemCount, int colorAmt, ImColor& col, ImColor& col2)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	UI::window = window;

	g.NextWindowData.ClearFlags();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);

	UI::AnimState& state = UI::anim[id];

	if (window->DC.CursorPos.y > (UI::mainWindow->Pos.y + UI::mainWindow->Size.y - labelSize.y))
	{
		state.opened = false;
		state.sliderPercent = 0.f;
		state.text = UI::textCol;
		state.filled = UI::bgColDark;
		return false;
	}

	auto pos = window->DC.CursorPos;

	const float squareSz = ImGui::GetFrameHeight();

	ImVec2 end(window->WorkRect.Max.x, pos.y + labelSize.y + style.FramePadding.y * 2.0f);
	const ImRect bb(pos, end);

	ImVec2 start2(end.x - squareSz * 6, pos.y);
	const ImRect comboBB(start2, end);

	ImGui::ItemSize(comboBB, style.FramePadding.y);

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(comboBB, id, &hovered, &held);

	if (hovered && g.IO.MouseClicked[0] || state.opened && g.IO.MouseClicked[0] && !state.hovered)
		state.opened = !state.opened;

	const auto draw = ImGui::GetForegroundDrawList();
	const auto draw2 = window->DrawList;

	state.filled = ImLerp(state.filled, state.sliderPercent > 2.f ? UI::bgColLight2 : hovered ? UI::bgColLight2 : UI::bgColDark, g.IO.DeltaTime * UI::animSpeed);
	state.text = ImLerp(state.text, pressed ? UI::textCol : hovered ? UI::textCol2 : UI::textCol, g.IO.DeltaTime * UI::animSpeed);
	state.sliderPercent = ImLerp(state.sliderPercent, state.opened ? CalcMaxPopupHeightFromItemCount(itemCount) : 0.f, g.IO.DeltaTime * UI::animSpeed);

	const ImVec2 textSize2 = ImGui::CalcTextSize(label);

	draw2->AddText(ImVec2(bb.Min.x, bb.GetCenter().y - textSize2.y / 2), ImGui::GetColorU32(state.text), label);
	draw2->AddRectFilled(comboBB.Min, ImVec2(comboBB.Max.x, comboBB.Max.y + state.sliderPercent), ImGui::GetColorU32(state.filled), style.FrameRounding);
	
	if (state.sliderPercent > 2.f)
		draw->AddRectFilled(comboBB.Min, ImVec2(comboBB.Max.x, comboBB.Max.y + state.sliderPercent), ImGui::GetColorU32(state.filled), style.FrameRounding);

	if (preview != NULL)
	{
		const ImVec2 textSize = ImGui::CalcTextSize(preview);

		if (state.sliderPercent > 2.f)
		{
			draw->PushClipRect(comboBB.Min, ImVec2(comboBB.Max.x - 5, comboBB.Max.y));
			draw->AddText(ImVec2(comboBB.Min.x + 5, comboBB.GetCenter().y - textSize.y / 2), ImGui::GetColorU32(state.text), preview);
			draw->PopClipRect();
		}
		else
		{
			draw2->PushClipRect(comboBB.Min, ImVec2(comboBB.Max.x - 5, comboBB.Max.y));
			draw2->AddText(ImVec2(comboBB.Min.x + 5, comboBB.GetCenter().y - textSize.y / 2), ImGui::GetColorU32(state.text), preview);
			draw2->PopClipRect();
		}
	}

	ImGui::ItemAdd(bb, id, &bb);

	if (!state.opened && state.sliderPercent < 20.f)
		return false;

	ImGui::SetNextWindowPos(ImVec2(comboBB.Min.x, comboBB.Max.y + 5));
	ImGui::SetNextWindowSize(ImVec2(comboBB.GetWidth(), state.sliderPercent));

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.000001f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

	ImGui::Begin(label, NULL, windowFlags);

	ImGui::PopStyleVar(3);

	state.hovered = ImGui::IsWindowHovered();

	return true;
}

static bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags = 0, const ImVec2& sizeArg = ImVec2(0, 0))
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImGuiID id = window->GetID(label);
	ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 size(sizeArg.x != 0.0f ? sizeArg.x : labelSize.x, sizeArg.y != 0.0f ? sizeArg.y : labelSize.y);
	ImVec2 pos = window->DC.CursorPos;
	pos.y += window->DC.CurrLineTextBaseOffset;
	ImGui::ItemSize(size, 0.0f);

	const bool spanAllColumns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
	const float minX = spanAllColumns ? window->ParentWorkRect.Min.x : pos.x;
	const float maxX = spanAllColumns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
	if (sizeArg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
		size.x = ImMax(labelSize.x, maxX - minX);

	const ImVec2 textMin = pos;
	const ImVec2 textMax(minX + size.x, pos.y + size.y);

	ImRect bb(minX, pos.y, textMax.x, textMax.y);
	if (!(flags & ImGuiSelectableFlags_NoPadWithHalfSpacing))
	{
		const float spacingX = spanAllColumns ? 0.0f : style.ItemSpacing.x;
		const float spacingY = style.ItemSpacing.y;
		const float spacingL = IM_TRUNC(spacingX * 0.50f);
		const float spacingU = IM_TRUNC(spacingY * 0.50f);
		bb.Min.x -= spacingL;
		bb.Min.y -= spacingU;
		bb.Max.x += (spacingX - spacingL);
		bb.Max.y += (spacingY - spacingU);
	}

	const bool disabledItem = (flags & ImGuiSelectableFlags_Disabled) != 0;
	const ImGuiItemFlags extraItemFlags = disabledItem ? (ImGuiItemFlags)ImGuiItemFlags_Disabled : ImGuiItemFlags_None;
	bool isVisible;
	if (spanAllColumns)
	{
		const float backupClipRectMinX = window->ClipRect.Min.x;
		const float backupClipRectMaxX = window->ClipRect.Max.x;
		window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
		window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
		isVisible = ImGui::ItemAdd(bb, id, NULL, extraItemFlags);
		window->ClipRect.Min.x = backupClipRectMinX;
		window->ClipRect.Max.x = backupClipRectMaxX;
	}
	else
		isVisible = ImGui::ItemAdd(bb, id, NULL, extraItemFlags);

	const bool isMultiSelect = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
	if (!isVisible)
		if (!isMultiSelect || !g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(bb))
			return false;

	const bool disabledGlobal = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
	if (disabledItem && !disabledGlobal)
		ImGui::BeginDisabled();

	if (spanAllColumns)
	{
		if (g.CurrentTable)
			ImGui::TablePushBackgroundChannel();
		else if (window->DC.CurrentColumns)
			ImGui::PushColumnsBackground();
		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
		g.LastItemData.ClipRect = window->ClipRect;
	}

	ImGuiButtonFlags buttonFlags = 0;
	if (flags & ImGuiSelectableFlags_NoHoldingActiveID)
		buttonFlags |= ImGuiButtonFlags_NoHoldingActiveId;
	if (flags & ImGuiSelectableFlags_NoSetKeyOwner)
		buttonFlags |= ImGuiButtonFlags_NoSetKeyOwner;
	if (flags & ImGuiSelectableFlags_SelectOnClick)
		buttonFlags |= ImGuiButtonFlags_PressedOnClick;
	if (flags & ImGuiSelectableFlags_SelectOnRelease)
		buttonFlags |= ImGuiButtonFlags_PressedOnRelease;
	if (flags & ImGuiSelectableFlags_AllowDoubleClick)
		buttonFlags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
	if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap))
		buttonFlags |= ImGuiButtonFlags_AllowOverlap;

	const bool wasSelected = selected;
	if (isMultiSelect)
		ImGui::MultiSelectItemHeader(id, &selected, &buttonFlags);

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, buttonFlags);

	if (!isMultiSelect)
	{
		if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
		{
			if (g.NavJustMovedToId == id)
				selected = pressed = true;
		}
	}
	else
		ImGui::MultiSelectItemFooter(id, &selected, &pressed);

	if (pressed)
	{
		ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb));
		ImGui::MarkItemEdited(id);
	}

	if (selected != wasSelected)
		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

	if (isVisible)
	{
		const bool highlighted = hovered || (flags & ImGuiSelectableFlags_Highlight);
		if (highlighted || selected)
		{
			ImU32 col;
			if (selected && !highlighted)
				col = ImGui::GetColorU32(ImLerp(ImGui::GetStyleColorVec4(ImGuiCol_Header), ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered), 0.5f));
			else
				col = ImGui::GetColorU32((held && highlighted) ? ImGuiCol_HeaderActive : highlighted ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		}
	}

	if (spanAllColumns)
	{
		if (g.CurrentTable)
			ImGui::TablePopBackgroundChannel();
		else if (window->DC.CurrentColumns)
			ImGui::PopColumnsBackground();
	}

	UI::AnimState& state = UI::anim[id];

	state.text = ImLerp(state.text, selected ? UI::primaryCol : hovered ? UI::textCol2 : UI::textCol, g.IO.DeltaTime * UI::animSpeed);

	ImGui::PushStyleColor(ImGuiCol_Text, state.text);

	const auto draw = ImGui::GetForegroundDrawList();

	if (isVisible)
		draw->AddText(textMin, ImGui::GetColorU32(state.text), label);

	ImGui::PopStyleColor();

	if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_NoAutoClosePopups) && (g.LastItemData.ItemFlags & ImGuiItemFlags_AutoClosePopups))
		ImGui::CloseCurrentPopup();

	if (disabledItem && !disabledGlobal)
		ImGui::EndDisabled();

	return pressed;
}

static bool Selectable2(const char* label, bool* pSelected, ImGuiSelectableFlags flags, const ImVec2& sizeArg = ImVec2(0, 0))
{
	if (Selectable(label, *pSelected, flags, sizeArg))
	{
		*pSelected = !*pSelected;
		return true;
	}
	return false;
}

void UI::Combo(const char* label, int* currentItem, const char* items[], int itemsCount)
{
	const char* preview = NULL;
	if (*currentItem >= 0 && *currentItem < itemsCount)
		preview = items[*currentItem];

	if (BaseCombo(label, preview, itemsCount, 0, d, d))
	{
		for (int i = 0; i < itemsCount; ++i)
		{
			const bool itemSelected = (i == *currentItem);

			if (Selectable(items[i], itemSelected, ImGuiSelectableFlags_DontClosePopups) && *currentItem != i)
			{
				*currentItem = i;
				const ImGuiID id = window->GetID(label);
				UI::AnimState& state = anim[id];
				state.opened = false;
			}
		}
		ImGui::End();
	}
}

void UI::Combo(const char* label, int* currentItem, const char* items[], int itemsCount, ImColor& col)
{
	const char* preview = NULL;
	if (*currentItem >= 0 && *currentItem < itemsCount)
		preview = items[*currentItem];

	if (BaseCombo(label, preview, itemsCount, 1, col, d))
	{
		for (int i = 0; i < itemsCount; ++i)
		{
			const bool itemSelected = (i == *currentItem);

			if (Selectable(items[i], itemSelected, ImGuiSelectableFlags_DontClosePopups) && *currentItem != i)
			{
				*currentItem = i;
				const ImGuiID id = window->GetID(label);
				UI::AnimState& state = anim[id];
				state.opened = false;
			}
		}
		ImGui::End();
	}
}

void UI::Combo(const char* label, int* currentItem, const char* items[], int itemsCount, ImColor& col, ImColor& col2)
{
	const char* preview = NULL;
	if (*currentItem >= 0 && *currentItem < itemsCount)
		preview = items[*currentItem];

	if (BaseCombo(label, preview, itemsCount, 2, col, col2))
	{
		for (int i = 0; i < itemsCount; ++i)
		{
			const bool itemSelected = (i == *currentItem);

			if (Selectable(items[i], itemSelected, ImGuiSelectableFlags_DontClosePopups) && *currentItem != i)
			{
				*currentItem = i;
				const ImGuiID id = window->GetID(label);
				UI::AnimState& state = anim[id];
				state.opened = false;
			}
		}
		ImGui::End();
	}
}

void UI::MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount)
{
	std::string preview = ("None");

	for (auto i = 0, j = 0; i < itemsCount; ++i)
	{
		if (variable[i])
		{
			if (j)
				preview += (", ") + (std::string)items[i];
			else
				preview = items[i];

			++j;
		}
	}

	if (BaseCombo(label, preview.c_str(), itemsCount, 0, d, d))
	{
		for (auto i = 0; i < itemsCount; ++i)
		{
			Selectable2(items[i], &variable[i], ImGuiSelectableFlags_DontClosePopups);
		}

		ImGui::End();
	}

	preview = ("None");
}


void UI::MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount, ImColor& col)
{
	std::string preview = ("None");

	for (auto i = 0, j = 0; i < itemsCount; ++i)
	{
		if (variable[i])
		{
			if (j)
				preview += (", ") + (std::string)items[i];
			else
				preview = items[i];

			++j;
		}
	}

	if (BaseCombo(label, preview.c_str(), itemsCount, 1, col, d))
	{
		for (auto i = 0; i < itemsCount; ++i)
		{
			Selectable2(items[i], &variable[i], ImGuiSelectableFlags_DontClosePopups);
		}

		ImGui::End();
	}

	preview = ("None");
}

void UI::MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount, ImColor& col, ImColor& col2)
{
	std::string preview = ("None");

	for (auto i = 0, j = 0; i < itemsCount; ++i)
	{
		if (variable[i])
		{
			if (j)
				preview += (", ") + (std::string)items[i];
			else
				preview = items[i];

			++j;
		}
	}

	if (BaseCombo(label, preview.c_str(), itemsCount, 2, col, col2))
	{
		for (auto i = 0; i < itemsCount; ++i)
		{
			Selectable2(items[i], &variable[i], ImGuiSelectableFlags_DontClosePopups);
		}

		ImGui::End();
	}

	preview = ("None");
}

static void ColorEditRestoreHS(const float* col, float* H, float* S, float* V)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.ColorEditCurrentID != 0);
	if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
		return;

	if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
		*H = g.ColorEditSavedHue;

	if (*V == 0.0f)
		*S = g.ColorEditSavedSat;
}

static void ColorEditRestoreH(const float* col, float* H)
{
	ImGuiContext& g = *GImGui;
	IM_ASSERT(g.ColorEditCurrentID != 0);
	if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
		return;
	*H = g.ColorEditSavedHue;
}

static bool ColorPickerPopup(const char* label, float col[4], ImGuiColorEditFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImDrawList* draw = window->DrawList;
	ImGuiStyle& style = g.Style;
	ImGuiIO& io = g.IO;

	const float width = ImGui::CalcItemWidth();
	g.NextItemData.ClearFlags();

	ImGui::PushID(label);
	const bool setCurrentColorEditID = (g.ColorEditCurrentID == 0);
	if (setCurrentColorEditID)
		g.ColorEditCurrentID = window->IDStack.back();
	ImGui::BeginGroup();

	if (!(flags & ImGuiColorEditFlags_NoSidePreview)) flags |= ImGuiColorEditFlags_NoSmallPreview;

	if (!(flags & ImGuiColorEditFlags_NoOptions)) ImGui::ColorPickerOptionsPopup(col, flags);

	if (!(flags & ImGuiColorEditFlags_PickerMask_)) flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
	if (!(flags & ImGuiColorEditFlags_InputMask_)) flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_));
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));
	if (!(flags & ImGuiColorEditFlags_NoOptions)) flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

	int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
	bool alphaBar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
	ImVec2 pickerPos = window->DC.CursorPos;
	float barsWidth = 15.f;
	float svPickerSize = ImMax(barsWidth * 1, width - (alphaBar ? 2 : 1) * (barsWidth));
	float bar0PosX = pickerPos.x + svPickerSize + 10;
	float bar1PosX = bar0PosX + barsWidth + 10;

	float backupInitialCol[4];
	memcpy(backupInitialCol, col, components * sizeof(float));

	float H = col[0], S = col[1], V = col[2];
	float R = col[0], G = col[1], B = col[2];
	if (flags & ImGuiColorEditFlags_InputRGB)
	{
		ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
		ColorEditRestoreHS(col, &H, &S, &V);
	}
	else if (flags & ImGuiColorEditFlags_InputHSV)
	{
		ImGui::ColorConvertHSVtoRGB(H, S, V, R, G, B);
	}

	bool valueChanged = false, valueChangedH = false, valueChangedSv = false;

	ImGui::InvisibleButton(("sv"), ImVec2(svPickerSize, svPickerSize));
	if (ImGui::IsItemActive())
	{
		S = ImSaturate((io.MousePos.x - pickerPos.x) / (svPickerSize - 1));
		V = 1.0f - ImSaturate((io.MousePos.y - pickerPos.y) / (svPickerSize - 1));
		ColorEditRestoreH(col, &H);
		valueChanged = valueChangedSv = true;
	}

	ImGui::SetCursorScreenPos(ImVec2(bar0PosX, pickerPos.y));
	ImGui::InvisibleButton(("hue"), ImVec2(barsWidth, svPickerSize));
	if (ImGui::IsItemActive())
	{
		H = ImSaturate((io.MousePos.y - pickerPos.y) / (svPickerSize - 1));
		valueChanged = valueChangedH = true;
	}

	if (alphaBar)
	{
		ImGui::SetCursorScreenPos(ImVec2(bar1PosX, pickerPos.y));
		ImGui::InvisibleButton(("alpha"), ImVec2(barsWidth, svPickerSize));
		if (ImGui::IsItemActive())
		{
			col[3] = 1.0f - ImSaturate((io.MousePos.y - pickerPos.y) / (svPickerSize - 1));
			valueChanged = true;
		}
	}

	if (!(flags & ImGuiColorEditFlags_NoLabel))
	{
		const char* labelDisplayEnd = ImGui::FindRenderedTextEnd(label);
		if (label != labelDisplayEnd)
		{
			if ((flags & ImGuiColorEditFlags_NoSidePreview))
				ImGui::SameLine(0, style.ItemInnerSpacing.x);
			ImGui::TextEx(label, labelDisplayEnd);
		}
	}

	if (valueChangedH || valueChangedSv)
	{
		if (flags & ImGuiColorEditFlags_InputRGB)
		{
			ImGui::ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
			g.ColorEditSavedHue = H;
			g.ColorEditSavedSat = S;
			g.ColorEditSavedID = g.ColorEditCurrentID;
			g.ColorEditSavedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
		}
		else if (flags & ImGuiColorEditFlags_InputHSV)
		{
			col[0] = H;
			col[1] = S;
			col[2] = V;
		}
	}

	if (valueChanged)
	{
		if (flags & ImGuiColorEditFlags_InputRGB)
		{
			R = col[0];
			G = col[1];
			B = col[2];
			ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
			ColorEditRestoreHS(col, &H, &S, &V);
		}
		else if (flags & ImGuiColorEditFlags_InputHSV)
		{
			H = col[0];
			S = col[1];
			V = col[2];
			ImGui::ColorConvertHSVtoRGB(H, S, V, R, G, B);
		}
	}

	if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		ImGui::PushItemWidth((alphaBar ? bar1PosX : bar0PosX) + barsWidth - pickerPos.x);
		ImGuiColorEditFlags subFlagsToForward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
		ImGuiColorEditFlags subFlags = (flags & subFlagsToForward) | ImGuiColorEditFlags_NoPicker;
		valueChanged |= ImGui::ColorEdit4(("##hex"), col, subFlags | ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_NoOptions);
		ImGui::PopItemWidth();
	}

	const int styleAlpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
	const ImU32 colBlack = IM_COL32(0, 0, 0, styleAlpha8);
	const ImU32 colWhite = IM_COL32(255, 255, 255, styleAlpha8);
	const ImU32 colHues[6 + 1] = { IM_COL32(255, 0, 0, styleAlpha8), IM_COL32(255, 255, 0, styleAlpha8), IM_COL32(0, 255, 0, styleAlpha8), IM_COL32(0, 255, 255, styleAlpha8), IM_COL32(0, 0, 255, styleAlpha8), IM_COL32(255, 0, 255, styleAlpha8), IM_COL32(255, 0, 0, styleAlpha8) };

	ImVec4 hueColorF(1, 1, 1, style.Alpha); ImGui::ColorConvertHSVtoRGB(H, 1, 1, hueColorF.x, hueColorF.y, hueColorF.z);
	ImU32 hueColor32 = ImGui::ColorConvertFloat4ToU32(hueColorF);
	ImU32 userCol32StrippedAlpha = ImGui::ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha));

	UI::AnimState& state = UI::anim[ImGui::GetID(label)];

	ImVec2 svCursorPos;
	ImVec2 pickerPosEnd = ImVec2(pickerPos.x + svPickerSize, pickerPos.y + svPickerSize);
	ImVec2 pickerPosStart2 = ImVec2(pickerPos.x - 1, pickerPos.y - 1);
	ImVec2 pickerPosEnd2 = ImVec2(pickerPos.x + svPickerSize + 1, pickerPos.y + svPickerSize + 1);

	draw->AddRectFilledMultiColorRounded(pickerPos, pickerPosEnd, colWhite, hueColor32, hueColor32, colWhite, UI::itemRounding);
	draw->AddRectFilledMultiColorRounded(pickerPosStart2, pickerPosEnd2, 0, 0, colBlack, colBlack, UI::itemRounding);

	svCursorPos.x = ImClamp(IM_ROUND(pickerPos.x + ImSaturate(S) * svPickerSize), pickerPos.x + 2, pickerPos.x + svPickerSize - 2);
	svCursorPos.y = ImClamp(IM_ROUND(pickerPos.y + ImSaturate(1 - V) * svPickerSize), pickerPos.y + 2, pickerPos.y + svPickerSize - 2);

	for (int i = 0; i < 6; ++i)
		draw->AddRectFilledMultiColorRounded(ImVec2(bar0PosX, pickerPos.y + i * (svPickerSize / 6) - (i == 5 ? 1 : 0)), ImVec2(bar0PosX + barsWidth, pickerPos.y + (i + 1) * (svPickerSize / 6) + (i == 0 ? 1 : 0)), colHues[i], colHues[i], colHues[i + 1], colHues[i + 1], style.FrameRounding, i == 0 ? ImDrawFlags_RoundCornersTop : i == 5 ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

	float bar0LineY = IM_ROUND(pickerPos.y + H * svPickerSize);
	bar0LineY = ImClamp(bar0LineY, pickerPos.y + 3.f, pickerPos.y + (svPickerSize - 13));

	state.hue = ImLerp(state.hue, bar0LineY + 5, g.IO.DeltaTime * (UI::animSpeed * 2));
	draw->AddCircleFilled(ImVec2(bar0PosX + 7.5f, state.hue), 4.5f, ImColor(255, 255, 255), 100);
	draw->AddCircle(ImVec2(bar0PosX + 7.5f, state.hue), 4.5f, ImColor(0, 0, 0), 100);

	float svCursorRad = valueChangedSv ? 10.0f : 6.0f;
	int svCursorSegments = draw->_CalcCircleAutoSegmentCount(svCursorRad);

	state.circleMove = ImLerp(state.circleMove, svCursorPos, g.IO.DeltaTime * 10.f);
	state.circle = ImLerp(state.circle, valueChangedSv ? 6.0f : 4.0f, g.IO.DeltaTime * (UI::animSpeed * 2));

	draw->AddCircleFilled(state.circleMove, state.circle, colWhite, svCursorSegments);
	draw->AddCircle(state.circleMove, state.circle, ImColor(0, 0, 0), svCursorSegments);

	if (alphaBar)
	{
		float alpha = ImSaturate(col[3]);
		ImRect bar1BB(bar1PosX, pickerPos.y, bar1PosX + barsWidth, pickerPos.y + svPickerSize);

		draw->AddRectFilledMultiColorRounded(bar1BB.Min, bar1BB.Max, userCol32StrippedAlpha, userCol32StrippedAlpha, userCol32StrippedAlpha & ~IM_COL32_A_MASK, userCol32StrippedAlpha & ~IM_COL32_A_MASK, 100.f);

		float bar1LineY = IM_ROUND(pickerPos.y + (1.0f - alpha) * svPickerSize);
		bar1LineY = ImClamp(bar1LineY, pickerPos.y + 3.f, pickerPos.y + (svPickerSize - 13));

		state.alphaBar = ImLerp(state.alphaBar, bar1LineY + 5, g.IO.DeltaTime * (UI::animSpeed * 2));
		draw->AddCircleFilled(ImVec2(bar1PosX + 7.5f, state.alphaBar), 4.5f, ImColor(255, 255, 255), 100);
		draw->AddCircle(ImVec2(bar1PosX + 7.5f, state.alphaBar), 4.5f, ImColor(0, 0, 0), 100);
	}

	ImGui::EndGroup();

	if (valueChanged && memcmp(backupInitialCol, col, components * sizeof(float)) == 0)
		valueChanged = false;
	if (valueChanged)
		ImGui::MarkItemEdited(g.LastItemData.ID);

	ImGui::PopID();

	return valueChanged;
}

DWORD pickerFlags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip;

bool UI::ColorPicker(const char* label, ImColor& col, int amount, ImGuiColorEditFlags flags, bool isCombo)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);
	float wFull = ImGui::CalcItemWidth();
	const float squareSz = ImGui::GetFrameHeight();
	const float offset = isCombo ? squareSz * (6 + amount - 1) : squareSz * amount;

	ImVec2 end(window->WorkRect.Max.x - offset - ((style.ItemInnerSpacing.x / 2) * amount), pos.y + labelSize.y + style.FramePadding.y * 2.0f);
	ImVec2 start(end.x - squareSz - style.ItemInnerSpacing.x / 2, pos.y);
	const ImRect bb(start, end);

	if (flags == NULL)
		flags = pickerFlags;

	g.NextItemData.ClearFlags();

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	const bool setCurrentColorEditID = (g.ColorEditCurrentID == 0);
	if (setCurrentColorEditID)
		g.ColorEditCurrentID = window->IDStack.back();

	const ImGuiColorEditFlags flags_untouched = flags;
	if (flags & ImGuiColorEditFlags_NoInputs)
		flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

	if (!(flags & ImGuiColorEditFlags_NoOptions))
		ImGui::ColorEditOptionsPopup((float*)&col, flags);

	if (!(flags & ImGuiColorEditFlags_DisplayMask_))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
	if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
	if (!(flags & ImGuiColorEditFlags_PickerMask_))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
	if (!(flags & ImGuiColorEditFlags_InputMask_))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
	flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_));
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));

	const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
	const float wButton = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (squareSz + style.ItemInnerSpacing.x);
	const float wInputs = ImMax(wFull - wButton, 1.0f);
	wFull = wInputs + wButton;

	float f[4] = { col.Value.x, col.Value.y, col.Value.z, col.Value.w };
	if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
		ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
	else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
	{
		ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
		ColorEditRestoreHS((float*)&col, &f[0], &f[1], &f[2]);
	}
	int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

	bool valueChanged = false;
	bool valueChangedAsFloat = false;


	AnimState& state = anim[id];

	ImGuiWindow* pickerActiveWindow = NULL;
	char buf[64];
	static bool searchCol = false;

	std::string newLabel = "##" + std::string(label);

	if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
	{
		const ImVec4 colV4(col);
		window->DC.CursorPos = start;
		ImGui::ColorButton((newLabel + ("button")).c_str(), colV4, flags, ImVec2(squareSz, squareSz));
		if (((ImGui::IsMouseHoveringRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max) && g.IO.MouseClicked[0]) || (state.active && !searchCol && g.IO.MouseClicked[0] && !state.hovered)))
			state.active = !state.active;

		state.alphaBar = ImLerp(state.alphaBar, searchCol ? 1.f : 0.f, g.IO.DeltaTime * UI::animSpeed);
		state.alpha = ImClamp(state.alpha + (8.f * g.IO.DeltaTime * (state.active ? 1.f : -1.f)), 0.f, 1.f);
		state.menuMod = ImLerp(state.menuMod, state.active ? 1.f : 0.f, g.IO.DeltaTime * UI::animSpeed);

		if (!state.active)
			state.menuMod = 0.f;

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, state.alpha);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
		if (state.active)
		{
			ImGui::SetNextWindowPos(g.LastItemData.Rect.GetTR());

			ImGui::Begin((newLabel + ("pickerwindow")).c_str(), NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				state.hovered = ImGui::IsWindowHovered();

				if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
					searchCol = false;

				if (alpha)
					ImFormatString(buf, IM_ARRAYSIZE(buf), ("#%02X%02X%02X%02X"), ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
				else
					ImFormatString(buf, IM_ARRAYSIZE(buf), ("#%02X%02X%02X"), ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));

				pickerActiveWindow = g.CurrentWindow;
				ImGuiColorEditFlags pickerFlagsToForward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
				ImGuiColorEditFlags pickerFlags2 = (flags_untouched & pickerFlagsToForward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;

				ImGui::SetNextItemWidth((squareSz * 10.f) * state.menuMod);
				valueChanged |= ColorPickerPopup((newLabel + ("picker")).c_str(), (float*)&col, pickerFlags2);
			}
			ImGui::End();
		}
		ImGui::PopStyleVar(3);
	}

	if (valueChanged && pickerActiveWindow == NULL)
	{
		if (!valueChangedAsFloat)
			for (int n = 0; n < 4; ++n)
				f[n] = i[n] / 255.0f;
		if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
		{
			g.ColorEditSavedHue = f[0];
			g.ColorEditSavedSat = f[1];
			ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
			g.ColorEditSavedID = g.ColorEditCurrentID;
			g.ColorEditSavedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
		}
		if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
			ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

		col.Value.x = f[0];
		col.Value.y = f[1];
		col.Value.z = f[2];
		if (alpha)
			col.Value.w = f[3];
	}

	if (setCurrentColorEditID)
		g.ColorEditCurrentID = 0;

	if (pickerActiveWindow && g.ActiveId != 0 && g.ActiveIdWindow == pickerActiveWindow)
		g.LastItemData.ID = g.ActiveId;

	if (valueChanged && g.LastItemData.ID != 0)
		ImGui::MarkItemEdited(g.LastItemData.ID);

	return valueChanged;
}

void UI::ToolTip(const char* tooltip)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiIO& io = ImGui::GetIO();
	const auto id = window->GetID(tooltip);

	AnimState& state = anim[id];

	if (!ImGui::IsItemHovered())
		state.alphaBar = ImLerp(state.alphaBar, 0.f, (UI::animSpeed * 1.5f) * io.DeltaTime);
	else 
		state.alphaBar = ImLerp(state.alphaBar, 1.f, (UI::animSpeed * 1.5f) * io.DeltaTime);

	if (!ImGui::IsItemHovered())
		state.alpha = ImLerp(state.alpha, 0.f, (UI::animSpeed * 1.5f) * io.DeltaTime);
	else if (state.alphaBar >= 0.99f)
		state.alpha = ImLerp(state.alpha, 1.f, UI::animSpeed * io.DeltaTime);

	if (state.alpha <= 0.01f)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, state.alpha);
	ImGui::SetTooltip(tooltip);
	ImGui::PopStyleVar();
}

bool UI::LoadingCircle(const char* label, float radius, int thickness, const ImU32& color, ImVec2 pos)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	pos = ImVec2(pos.x - radius, pos.y - radius);

	ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

	window->DrawList->PathClear();

	const float time = float(g.Time);
	const int segments = 200;
	const int start = int(abs(ImSin(time * 1.8f) * (segments - 5)));

	const float a_min = IM_PI * 2.0f * ((float)start) / (float)segments;
	const float a_max = IM_PI * 2.0f * ((float)segments - 3) / (float)segments;

	const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

	for (int i = 0; i < segments; ++i)
	{
		const float a = a_min + ((float)i / (float)segments) * (a_max - a_min);
		window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + time * 8.f) * radius,
			centre.y + ImSin(a + time * 8.f) * radius));
	}

	window->DrawList->PathStroke(color, false, float(thickness));
	return true;
}

/* very stupid way of doing things, this would be unnecessary on newer imgui versions
but on this one changing font sizes without initializing them on that size makes them blurry
so u have to init a font for each size u want to use */
void UI::LoadFonts(ImGuiIO& io)
{
	ImFontConfig fontCfg;
	fontCfg.FontDataOwnedByAtlas = false;

	if (UI::fonts.mainFont.tinyDefault == nullptr)
		UI::fonts.mainFont.tinyDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 8, &fontCfg);

	if (UI::fonts.mainFont.smallerDefault == nullptr)
		UI::fonts.mainFont.smallerDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 10, &fontCfg);
	
	if (UI::fonts.mainFont.smallDefault == nullptr)
		UI::fonts.mainFont.smallDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 12, &fontCfg);

	if (UI::fonts.mainFont.normalDefault == nullptr)
		UI::fonts.mainFont.normalDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 16, &fontCfg);

	if (UI::fonts.mainFont.bigDefault == nullptr)
		UI::fonts.mainFont.bigDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 20, &fontCfg);

	if (UI::fonts.mainFont.largeDefault == nullptr)
		UI::fonts.mainFont.largeDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 24, &fontCfg);

	if (UI::fonts.mainFont.hugeDefault == nullptr)
		UI::fonts.mainFont.hugeDefault = io.Fonts->AddFontFromMemoryTTF(interMedium, sizeof(interMedium), 28, &fontCfg);

	if (UI::fonts.mainFont.tinyBold == nullptr)
		UI::fonts.mainFont.tinyBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 8, &fontCfg);

	if (UI::fonts.mainFont.smallerBold == nullptr)
		UI::fonts.mainFont.smallerBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 10, &fontCfg);
	
	if (UI::fonts.mainFont.smallBold == nullptr)
		UI::fonts.mainFont.smallBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 12, &fontCfg);

	if (UI::fonts.mainFont.normalBold == nullptr)
		UI::fonts.mainFont.normalBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 16, &fontCfg);

	if (UI::fonts.mainFont.bigBold == nullptr)
		UI::fonts.mainFont.bigBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 20, &fontCfg);

	if (UI::fonts.mainFont.largeBold == nullptr)
		UI::fonts.mainFont.largeBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 24, &fontCfg);

	if (UI::fonts.mainFont.hugeBold == nullptr)
		UI::fonts.mainFont.hugeBold = io.Fonts->AddFontFromMemoryTTF(interBold, sizeof(interBold), 28, &fontCfg);

	if (fonts.iconFont.tinyDefault == nullptr)
		fonts.iconFont.tinyDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 12, &fontCfg);

	if (fonts.iconFont.smallerDefault == nullptr)
		fonts.iconFont.smallerDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 14, &fontCfg);

	if (fonts.iconFont.smallDefault == nullptr)
		fonts.iconFont.smallDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 16, &fontCfg);

	if (fonts.iconFont.normalDefault == nullptr)
		fonts.iconFont.normalDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 20, &fontCfg);

	if (fonts.iconFont.bigDefault == nullptr)
		fonts.iconFont.bigDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 24, &fontCfg);

	if (fonts.iconFont.largeDefault == nullptr)
		fonts.iconFont.largeDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 28, &fontCfg);

	if (fonts.iconFont.hugeDefault == nullptr)
		fonts.iconFont.hugeDefault = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 32, &fontCfg);

	if (fonts.iconFont.hugeBold == nullptr)
		fonts.iconFont.hugeBold = io.Fonts->AddFontFromMemoryTTF(uiIcons, sizeof(uiIcons), 64, &fontCfg);
}