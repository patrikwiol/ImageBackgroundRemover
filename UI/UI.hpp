#pragma once

namespace UI
{
    enum EStage
    {
        STAGE_STARTING,
        STAGE_LOADING,
        STAGE_MAIN
    };

    inline bool menuOpen = false;
    inline bool initialized = false;
    inline ImGuiWindow* window = nullptr;
    inline ImGuiWindow* mainWindow = nullptr;

    inline constexpr int windowRounding = 10;
    inline constexpr int itemRounding = 8;

    inline ImVec4 bgColDark(15 / 255.f, 20 / 255.f, 30 / 255.f, 255 / 255.f);
    inline ImVec4 bgColDark2(20 / 255.f, 25 / 255.f, 35 / 255.f, 255 / 255.f);
    inline ImVec4 bgColDark3(25 / 255.f, 30 / 255.f, 40 / 255.f, 255 / 255.f);
    inline ImVec4 bgColDark4(30 / 255.f, 35 / 255.f, 45 / 255.f, 255 / 255.f);
    inline ImVec4 bgColDark5(35 / 255.f, 40 / 255.f, 50 / 255.f, 255 / 255.f);
    inline ImVec4 bgColLight(40 / 255.f, 45 / 255.f, 55 / 255.f, 255 / 255.f);
    inline ImVec4 bgColLight2(60 / 255.f, 65 / 255.f, 75 / 255.f, 255 / 255.f);
    inline ImVec4 scrollCol(70 / 255.f, 75 / 255.f, 85 / 255.f, 255 / 255.f);
    inline ImVec4 scrollCol2(80 / 255.f, 85 / 255.f, 95 / 255.f, 255 / 255.f);
    inline ImVec4 scrollCol3(90 / 255.f, 95 / 255.f, 105 / 255.f, 255 / 255.f);
    inline ImVec4 textCol(200 / 255.f, 205 / 255.f, 215 / 255.f, 255 / 255.f);
    inline ImVec4 textCol2(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f);
    inline ImVec4 primaryCol(50 / 255.f, 150 / 255.f, 255 / 255.f, 255 / 255.f);
    inline ImVec4 blackCol(0 / 255.f, 0 / 255.f, 0 / 255.f, 255 / 255.f);
    inline ImVec4 transCol(0 / 255.f, 0 / 255.f, 0 / 255.f, 0 / 255.f);
    inline constexpr ImVec4 redCol(255 / 255.f, 50 / 255.f, 50 / 255.f, 255 / 255.f);
    inline constexpr ImVec4 greenCol(40 / 255.f, 240 / 255.f, 45 / 255.f, 255 / 255.f);

    inline constexpr float animSpeed = 10.f;

    inline constexpr float loadingRadius = 25.f;

    struct Fonts
    {
        struct FontType
        {
            ImFont* tinyDefault = nullptr;
            ImFont* smallerDefault = nullptr;
            ImFont* smallDefault = nullptr;
            ImFont* normalDefault = nullptr;
            ImFont* bigDefault = nullptr;
            ImFont* largeDefault = nullptr;
            ImFont* hugeDefault = nullptr;
            ImFont* tinyBold = nullptr;
            ImFont* smallerBold = nullptr;
            ImFont* smallBold = nullptr;
            ImFont* normalBold = nullptr;
            ImFont* bigBold = nullptr;
            ImFont* largeBold = nullptr;
            ImFont* hugeBold = nullptr;
        };

        FontType mainFont;
        FontType iconFont;
        int size = 16;
    }inline fonts;

    inline void CustomStyle()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        style->WindowRounding = windowRounding;
        style->ChildRounding = itemRounding;
        style->FrameRounding = itemRounding;
        style->PopupRounding = itemRounding;
        style->ScrollbarRounding = itemRounding;
        style->ScrollbarSize = fonts.size == 16 ? 14.f : fonts.size == 12 ? 12.f : 17.f;

        colors[ImGuiCol_Text] = textCol;
        colors[ImGuiCol_TextDisabled] = textCol;
        colors[ImGuiCol_WindowBg] = bgColDark2;
        colors[ImGuiCol_ChildBg] = bgColDark;
        colors[ImGuiCol_PopupBg] = bgColDark2;
        colors[ImGuiCol_Border] = bgColLight;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = bgColDark3;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_TitleBg] = bgColDark;
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.0f);
        colors[ImGuiCol_ScrollbarGrab] = scrollCol;
        colors[ImGuiCol_ScrollbarGrabHovered] = scrollCol2;
        colors[ImGuiCol_ScrollbarGrabActive] = scrollCol3;
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button] = bgColDark;
        colors[ImGuiCol_ButtonHovered] = bgColLight;
        colors[ImGuiCol_ButtonActive] = primaryCol;
        colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = textCol2;
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_Tab] = colors[ImGuiCol_Header];
        colors[ImGuiCol_TabSelected] = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TabDimmed] = colors[ImGuiCol_Tab];
        colors[ImGuiCol_TabDimmedSelected] = colors[ImGuiCol_TabSelected];
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.f);
        colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TextSelectedBg] = ImVec4(primaryCol.x, primaryCol.y, primaryCol.z, 0.7f);
        colors[ImGuiCol_DragDropTarget] = primaryCol;
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    struct AnimState
    {
        ImVec4 text, text2, filled, filled2, checkMark, shadowOpacity, icon, icon2, icon3, icon4, iconFilled, iconFilled2, iconFilled3, iconFilled4;
        ImVec2 circleMove, pos, pos2;
        float sliderPercent = 0.f, alpha = 0.f, alphaBar = 0.f, menuMod = 0.f, circle = 0.f, hue = 0.f;
        bool hovered, active, opened;
    };

    inline AnimState state{};
    inline std::unordered_map<ImGuiID, AnimState> anim{};

    struct MenuData
    {
        bool shouldExit = false;
        float alpha = 0.f;
        ImVec2 screenSize{};
        ImVec2 windowPos{};
        ImVec2 windowSize = ImVec2(1200.f, 880.f);
        int stage = STAGE_STARTING;
    }inline data;

    void Header(const char* text);
    void BeginChild(const char* text, ImVec2 childSize, ImGuiChildFlags flags);
    void EndChild();
    bool Checkbox(const char* label, bool* v);
    bool Checkbox(const char* label, bool* v, ImColor& col);
    bool Checkbox(const char* label, bool* v, ImColor& col, ImColor& col2);
    bool SliderInt(const char* label, int* v, int vMin, int vMax, const char* format = "%d", const char* addon = "");
    bool SliderFloat(const char* label, float* v, float vMin, float vMax, const char* format = "%.1f");
    void Combo(const char* label, int* currentItem, const char* items[], int itemsCount);
    void Combo(const char* label, int* currentItem, const char* items[], int itemsCount, ImColor& col);
    void Combo(const char* label, int* currentItem, const char* items[], int itemsCount, ImColor& col, ImColor& col2);
    void MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount);
    void MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount, ImColor& col);
    void MultiCombo(const char* label, bool variable[], const char* items[], int itemsCount, ImColor& col, ImColor& col2);
    bool ColorPicker(const char* label, ImColor& col, int amount = 0, ImGuiColorEditFlags flags = NULL, bool isCombo = false);
    void ToolTip(const char* tooltip);
    bool LoadingCircle(const char* label, float radius, int thickness, const ImU32& color, ImVec2 pos = ImVec2(0, 0));
    void LoadFonts(ImGuiIO& io);
}