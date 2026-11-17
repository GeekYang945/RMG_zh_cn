/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2026 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "OnScreenDisplay.hpp"

#include <RMG-Core/Settings.hpp>

#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <chrono>
#include <filesystem>
#include <cstdlib>
#include <atomic>
#include <vector>

//
// Local Variables
//

static bool l_Initialized     = false;
static bool l_Enabled         = false;
static bool l_RenderingPaused = false;

static std::string l_Language;

static std::atomic<int> l_DisplayWidth{0};
static std::atomic<int> l_DisplayHeight{0};

static std::chrono::time_point<std::chrono::high_resolution_clock> l_MessageTime;
static std::string l_Message;
static int         l_MessagePosition = 1;
static float       l_MessagePaddingX = 20.0f;
static float       l_MessagePaddingY = 20.0f;
static float       l_BackgroundRed   = 1.0f;
static float       l_BackgroundGreen = 1.0f;
static float       l_BackgroundBlue  = 1.0f;
static float       l_BackgroundAlpha = 1.0f;
static float       l_TextRed         = 1.0f;
static float       l_TextGreen       = 1.0f;
static float       l_TextBlue        = 1.0f;
static float       l_TextAlpha       = 1.0f;
static int         l_MessageDuration = 3;

//
// Local Functions
//

// returns true when the currently configured language
// needs a font which contains CJK glyphs
static bool language_needs_cjk_font(void)
{
    return l_Language.rfind("zh", 0) == 0 ||
           l_Language.rfind("ja", 0) == 0 ||
           l_Language.rfind("ko", 0) == 0;
}

// returns the path of a system font which contains
// CJK glyphs, or an empty string when none was found
static std::string get_cjk_font_path(void)
{
    std::vector<std::string> fontCandidates;

#ifdef _WIN32
    const char* systemRoot = std::getenv("SystemRoot");
    const std::string windowsDirectory = (systemRoot != nullptr) ? systemRoot : "C:/Windows";
    fontCandidates =
    {
        windowsDirectory + "/Fonts/msyh.ttc",  // Microsoft YaHei
        windowsDirectory + "/Fonts/msyh.ttf",  // Microsoft YaHei (older)
        windowsDirectory + "/Fonts/msyhl.ttc", // Microsoft YaHei Light
        windowsDirectory + "/Fonts/simhei.ttf",// SimHei
        windowsDirectory + "/Fonts/simsun.ttc",// SimSun
        windowsDirectory + "/Fonts/Deng.ttf",  // DengXian
        windowsDirectory + "/Fonts/msjh.ttc",  // Microsoft JhengHei
    };
#elif defined(__APPLE__)
    fontCandidates =
    {
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STHeiti Light.ttc",
        "/System/Library/Fonts/Hiragino Sans GB.ttc",
    };
#else
    fontCandidates =
    {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJKsc-Regular.otf",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/google-noto-cjk/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/arphic/uming.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    };
#endif // _WIN32

    for (const std::string& fontPath : fontCandidates)
    {
        std::error_code errorCode;
        if (std::filesystem::is_regular_file(fontPath, errorCode))
        {
            return fontPath;
        }
    }

    return std::string();
}

//
// Exported Functions
//

bool OnScreenDisplayInit(void)
{
    if (l_Initialized)
    {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // the default ImGui font doesn't contain CJK glyphs,
    // so load a system font which contains them when the
    // configured language needs them
    if (language_needs_cjk_font())
    {
        const std::string fontPath = get_cjk_font_path();
        if (!fontPath.empty())
        {
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, nullptr,
                                         io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        }
    }

    if (!ImGui_ImplOpenGL3_Init())
    {
        return false;
    }

    l_Initialized = true;
    return true;
}

void OnScreenDisplayShutdown(void)
{
    if (!l_Initialized)
    {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    l_Message         = "";
    l_Initialized     = false;
    l_RenderingPaused = false;
}

void OnScreenDisplayLoadSettings(void)
{
    l_Enabled         = CoreSettingsGetBoolValue(SettingsID::GUI_OnScreenDisplayEnabled);
    l_MessagePosition = CoreSettingsGetIntValue(SettingsID::GUI_OnScreenDisplayLocation);
    l_MessagePaddingX = CoreSettingsGetIntValue(SettingsID::GUI_OnScreenDisplayPaddingX);
    l_MessagePaddingY = CoreSettingsGetIntValue(SettingsID::GUI_OnScreenDisplayPaddingY);
    l_MessageDuration = CoreSettingsGetIntValue(SettingsID::GUI_OnScreenDisplayDuration);

    std::vector<int> backgroundColor = CoreSettingsGetIntListValue(SettingsID::GUI_OnScreenDisplayBackgroundColor);
    std::vector<int> textColor       = CoreSettingsGetIntListValue(SettingsID::GUI_OnScreenDisplayTextColor);
    if (backgroundColor.size() == 4)
    {
        l_BackgroundRed   = backgroundColor.at(0) / 255.0f;
        l_BackgroundGreen = backgroundColor.at(1) / 255.0f;
        l_BackgroundBlue  = backgroundColor.at(2) / 255.0f;
        l_BackgroundAlpha = backgroundColor.at(3) / 255.0f;
    }
    if (textColor.size() == 4)
    {
        l_TextRed   = textColor.at(0) / 255.0f;
        l_TextGreen = textColor.at(1) / 255.0f;
        l_TextBlue  = textColor.at(2) / 255.0f;
        l_TextAlpha = textColor.at(3) / 255.0f;
    }
}

void OnScreenDisplaySetLanguage(std::string language)
{
    l_Language = language;
}

bool OnScreenDisplaySetDisplaySize(int width, int height)
{
    // the actual display size is applied
    // in OnScreenDisplayRender() before a new frame is started,
    // this can be called from another thread
    l_DisplayWidth  = width;
    l_DisplayHeight = height;
    return l_Initialized;
}

void OnScreenDisplaySetMessage(std::string message)
{
    if (!l_Initialized)
    {
        return;
    }

    l_Message     = message;
    l_MessageTime = std::chrono::high_resolution_clock::now();
}

void OnScreenDisplayRender(void)
{
    if (!l_Initialized || !l_Enabled || l_RenderingPaused || l_Message.empty())
    {
        return;
    }

    const auto currentTime  = std::chrono::high_resolution_clock::now();
    const int secondsPassed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - l_MessageTime).count();
    if (secondsPassed >= l_MessageDuration)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // use the actual size of the render surface
    // for positioning the OSD, the size provided
    // by the video plugin is not necessarily
    // the size of the surface in fullscreen
    const int displayWidth  = l_DisplayWidth.load();
    const int displayHeight = l_DisplayHeight.load();
    if (displayWidth > 0 && displayHeight > 0)
    {
        io.DisplaySize = ImVec2((float)displayWidth, (float)displayHeight);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // right bottom = ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 20.0f, io.DisplaySize.y - 20.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    // right top    = ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 20.0f, 20.0f), ImGuiCond_Always, ImVec2(1.0f, 0));
    // left  bottom = ImGui::SetNextWindowPos(ImVec2(20.0f, io.DisplaySize.y - 20.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    // left  top    = ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    switch (l_MessagePosition)
    {
    default:
    case 0: // left bottom
        ImGui::SetNextWindowPos(ImVec2(l_MessagePaddingX, io.DisplaySize.y - l_MessagePaddingY), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        break;
    case 1: // left top
        ImGui::SetNextWindowPos(ImVec2(l_MessagePaddingX, l_MessagePaddingY), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        break;
    case 2: // right top
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - l_MessagePaddingX, l_MessagePaddingY), ImGuiCond_Always, ImVec2(1.0f, 0));
        break;
    case 3: // right bottom
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - l_MessagePaddingX, io.DisplaySize.y - l_MessagePaddingY), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        break;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(l_BackgroundRed, l_BackgroundGreen, l_BackgroundBlue, l_BackgroundAlpha));
    ImGui::PushStyleColor(ImGuiCol_Text,     ImVec4(l_TextRed, l_TextGreen, l_TextBlue, l_TextAlpha));

    ImGui::Begin("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::Text("%s", l_Message.c_str());
    ImGui::End();

    ImGui::PopStyleColor(2);

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OnScreenDisplayPause(void)
{
    l_RenderingPaused = true;
}

void OnScreenDisplayResume(void)
{
    l_RenderingPaused = false;
}
