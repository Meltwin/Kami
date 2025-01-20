#include "kami/app/plugin/plugin_manager_impl.hpp"
#include "kami/app/ui/main_ui.hpp"
#include "kami/app/ui/rendering.hpp"
#include "kami/res/kami_icon.hpp"
#include "kami/utils/img.hpp"

#include <imgui_internal.h>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

// Main code
int main(int, char **) {
  const auto logger = Logger("Kami App");
  SETUP_RENDER
  ImGui::StyleColorsDark();

  auto plugin_manager = PluginManager::init();
  plugin_manager->load_plugins();

  // Loading app icon
  SDL_Surface *app_icon;
  if (app_icon = makeImage(KamiIcon); app_icon == nullptr)
    logger.error("Error while loading app icon %s", SDL_GetError());
  SDL_SetWindowIcon(window, app_icon);

  // Main loop
  bool done = false;
  while (!done) {
    auto app = plugin_manager->app();
    // Proces events
    SDL_Event event;
    bool idle = true;
    while (SDL_PollEvent(&event)) {
      idle = false;
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }
    if (idle) {
    }
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    // Call UI components
    START_FRAME;
    ImVec2 winsize;

    // Menu Bar
    {
      set_main_menu_style();
      ImGui::BeginMainMenuBar();
      ImGui::SetWindowSize(ImVec2(context->io.DisplaySize.x, 70));
      app->gui()->render_main_menu();
      winsize = ImGui::GetWindowSize();
      APPLY_STYLE
      ImGui::EndMainMenuBar();
    }

    // Main area
    const float MA_width = MA_WIDTH * context->io.DisplaySize.x - BORDER_WIDTH;
    const auto MA_pos = ImVec4(0, winsize.y + BORDER_WIDTH, MA_width,
                               context->io.DisplaySize.y);
    {
      set_MA_style();
      ImGui::Begin("Main area", nullptr,
                   LAYOUT_FLAGS | ImGuiWindowFlags_NoScrollbar);
      ImGui::SetWindowSize(
          ImVec2(MA_pos.z, MA_pos.w - winsize.y - BORDER_WIDTH));
      ImGui::SetWindowPos(ImVec2(MA_pos.x, MA_pos.y));
      APPLY_STYLE
      app->gui()->MA_render(MA_pos);
      ImGui::End();
    }

    // Second area
    {
      const auto SA_pos = ImVec4(MA_pos.z + BORDER_WIDTH, MA_pos.y,
                                 context->io.DisplaySize.x, MA_pos.w);
      set_SA_style();
      ImGui::Begin("Second area", nullptr, LAYOUT_FLAGS);
      ImGui::SetWindowSize(
          ImVec2(SA_WIDTH * context->io.DisplaySize.x,
                 context->io.DisplaySize.y - winsize.y - BORDER_WIDTH));
      ImGui::SetWindowPos(ImVec2(SA_pos.x, SA_pos.y));
      APPLY_STYLE
      app->gui()->SA_render(SA_pos);
      ImGui::End();
    }

    RENDER_FRAME
    std::this_thread::sleep_for((idle) ? 100ms : 10ms);
  }

  SDL_FreeSurface(app_icon);

  // Cleanup
  cleanup_rendering(context, window);
  return 0;
}