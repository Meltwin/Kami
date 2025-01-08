//
// Created by meltwin on 07/01/25.
//

#ifndef HUB_UI_HPP
#define HUB_UI_HPP

#include "kami/app/ui/BaseAppUI.hpp"

struct HubUI final : BaseAppUI {
  void MM_render_file() override;
  void MM_render_edit() override;
  void MA_custom_render() override;
  void SA_custom_render() override;
};

#endif // HUB_UI_HPP
