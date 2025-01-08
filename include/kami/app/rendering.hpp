//
// Created by meltwin on 07/01/25.
//

#ifndef RENDERING_HPP
#define RENDERING_HPP

#include "backends/imgui_impl_opengl2.h"
#include "backends/imgui_impl_sdl2.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#include <imgui.h>

#include "kami/app/app_info.hpp"

#include <memory>

struct AppContext {
  SDL_GLContext context;
  ImGuiIO &io;

  explicit AppContext(const SDL_GLContext &_context, ImGuiIO &_io)
      : context(_context), io(_io) {}

  typedef std::shared_ptr<AppContext> SharedPtr;
};

/*
 ==============================================================================
 Definition of rendering related functions
 ==============================================================================
 */

inline void setup_rendering() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    exit(-1);
  }

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Setup window
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
}

inline SDL_Window *create_window() {
  return SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT,
                          WIN_FLAGS);
}

inline AppContext::SharedPtr create_context(SDL_Window *window) {
  // Setup SDL context
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL2_Init();
  return std::make_shared<AppContext>(gl_context, io);
}

#define SETUP_RENDER                                                           \
  setup_rendering();                                                           \
  const auto window = create_window();                                         \
  if (window == nullptr) {                                                     \
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());                 \
    return -1;                                                                 \
  }                                                                            \
  const AppContext::SharedPtr context = create_context(window);

inline void cleanup_rendering(const AppContext::SharedPtr &app_context,
                              SDL_Window *window) {
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(app_context->context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

#define START_FRAME                                                            \
  ImGui_ImplOpenGL2_NewFrame();                                                \
  ImGui_ImplSDL2_NewFrame();                                                   \
  ImGui::NewFrame();

#define RENDER_FRAME                                                           \
  ImGui::Render();                                                             \
  draw_main_background(context->io);                                           \
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());                      \
  SDL_GL_SwapWindow(window);

#endif // RENDERING_HPP
