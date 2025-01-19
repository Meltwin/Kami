//
// Created by meltwin on 07/01/25.
//

#ifndef BASE_APP_UI_HPP
#define BASE_APP_UI_HPP

#include <imgui.h>

struct BaseAppUIMainMenuState {};

/**
 * Interface to declare function to render the UI
 */
struct BaseAppUI {

  // ================================================================
  // Main Menu
  // ================================================================

  /**
   * Render the main menu of the application
   */
public:
  virtual ~BaseAppUI() = default;
  void render_main_menu();

protected:
  virtual void MM_render_file() = 0;
  virtual void MM_render_edit() = 0;

  // ================================================================
  // Main area
  // ================================================================
public:
  void MA_render(const ImVec4 &);

protected:
  virtual void MA_custom_render() = 0;

  // ================================================================
  // Secondary area
  // ================================================================
public:
  void SA_render(const ImVec4 &);

protected:
  virtual void SA_custom_render() = 0;
};

#endif // BASE_APP_UI_HPP
