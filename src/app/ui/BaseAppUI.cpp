//
// Created by meltwin on 07/01/25.
//
#include "kami/app/ui/BaseAppUI.hpp"

#include <kami/app/app_info.hpp>

void BaseAppUI::render_main_menu() {
  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem("New");
    ImGui::MenuItem("Open");
    MM_render_file();
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Edit")) {
    MM_render_edit();
    ImGui::EndMenu();
  }
}

void BaseAppUI::MA_render(const ImVec4 &MA_pos) {
  const auto &io = ImGui::GetIO();

  // FPS Counter
  {
    static auto appname_line_size = ImGui::CalcTextSize(APP_NAME);
    static auto fps_line_size = ImGui::CalcTextSize("###.0 FPS");
    ImGui::SetCursorScreenPos(
        ImVec2{MA_pos.z - appname_line_size.x - APP_INFO_MARGIN.x,
               MA_pos.w - 2 * appname_line_size.y - APP_INFO_MARGIN.y});
    ImGui::TextColored(DEBUG_TEXT_COLOR, APP_NAME);
    ImGui::SetCursorScreenPos(
        ImVec2{MA_pos.z - fps_line_size.x - APP_INFO_MARGIN.x,
               MA_pos.w - fps_line_size.y - APP_INFO_MARGIN.y});
    ImGui::TextColored(DEBUG_TEXT_COLOR, "%5.1f FPS", io.Framerate);
    ImGui::SetCursorScreenPos(ImVec2{MA_pos.x, MA_pos.y});
  }
  MA_custom_render();
}

void BaseAppUI::SA_render(const ImVec4 &SA_pos) { SA_custom_render(); }
