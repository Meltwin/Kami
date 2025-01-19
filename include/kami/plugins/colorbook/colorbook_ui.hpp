#ifndef COLOR_BOOK_UI_HPP
#define COLOR_BOOK_UI_HPP

#include <kami/app/ui/BaseAppUI.hpp>

struct ColorBookUI final : BaseAppUI {
protected:
  void MM_render_file() override;
  void MM_render_edit() override;
  void MA_custom_render() override;
  void SA_custom_render() override;
};

#endif // COLOR_BOOK_UI_HPP
