//
// Created by meltwin on 07/01/25.
//

#ifndef MAIN_UI_HPP
#define MAIN_UI_HPP

#include "kami/app/app_info.hpp"
#include <SDL_opengl.h>
#include <imgui.h>

inline void draw_main_background(const ImGuiIO &io) {
  glViewport(0, 0, static_cast<int>(io.DisplaySize.x),
             static_cast<int>(io.DisplaySize.y));
  glClearColor(BACK_COLOR.x * BACK_COLOR.w, BACK_COLOR.y * BACK_COLOR.w,
               BACK_COLOR.z * BACK_COLOR.w, BACK_COLOR.w);
  glClear(GL_COLOR_BUFFER_BIT);
}

inline void set_main_menu_style() {
  ImGui::PushStyleColor(ImGuiCol_WindowBg, MA_BACK_COLOR);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
}

inline void set_MA_style() {
  ImGui::PushStyleColor(ImGuiCol_WindowBg, MA_BACK_COLOR);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
}

inline void set_SA_style() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, SA_BACK_COLOR);
}

#define APPLY_STYLE                                                            \
  ImGui::PopStyleColor();                                                      \
  ImGui::PopStyleVar();

#endif // MAIN_UI_HPP
