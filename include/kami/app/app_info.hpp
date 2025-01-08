//
// Created by meltwin on 07/01/25.
//

#ifndef APP_INFO_HPP
#define APP_INFO_HPP

#include "kami/common.hpp"
#include <SDL.h>
#include <imgui.h>

// Window settings
constchar APP_NAME{"Kami v2.0.0"};
constchar APP_VERSION{"v2.0.0"};
constint WIN_WIDTH{1280};
constint WIN_HEIGHT{720};
constexpr auto WIN_FLAGS{static_cast<SDL_WindowFlags>(
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI |
    SDL_WINDOW_MAXIMIZED)};
constexpr float BACK_GRAY_LVL{0.5};
constexpr ImVec4 BACK_COLOR{BACK_GRAY_LVL, BACK_GRAY_LVL, BACK_GRAY_LVL, 1};

// Area settings
constexpr int BORDER_WIDTH{1};
constexpr auto LAYOUT_FLAGS{
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse};
constexpr float MA_WIDTH{0.7};
constexpr float MA_GRAY_LVL{.075};
constexpr ImVec4 MA_BACK_COLOR{MA_GRAY_LVL, MA_GRAY_LVL, MA_GRAY_LVL, 1.0};
constexpr float SA_WIDTH{1 - MA_WIDTH};
constexpr float SA_GRAY_LVL{0.15};
constexpr ImVec4 SA_BACK_COLOR{SA_GRAY_LVL, SA_GRAY_LVL, SA_GRAY_LVL, 1.0};

// Other text settings
constexpr float FONT_SIZE{3};
constexpr float DEBUG_TEXT_GLVL{0.5};
constexpr ImVec4 DEBUG_TEXT_COLOR{DEBUG_TEXT_GLVL, DEBUG_TEXT_GLVL,
                                  DEBUG_TEXT_GLVL, 1};
constexpr ImVec2 APP_INFO_MARGIN{10, 5};
#endif // APP_INFO_HPP
