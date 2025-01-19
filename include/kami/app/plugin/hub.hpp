//
// Created by meltwin on 07/01/25.
//

#ifndef HUB_UI_HPP
#define HUB_UI_HPP

#include <kami/app/plugin/plugin.hpp>
#include <kami/app/ui/BaseAppUI.hpp>
#include <utility>

struct HubUI final : BaseAppUI {
  HubUI() = default;
  explicit HubUI(std::weak_ptr<PluginDescriptorList> l)
      : applications(std::move(l)) {}

  void MM_render_file() override;
  void MM_render_edit() override;
  void MA_custom_render() override;
  void SA_custom_render() override;

private:
  std::weak_ptr<PluginDescriptorList> applications;
};

struct Hub final : Plugin {
  std::shared_ptr<BaseAppUI> gui() override { return _gui; }

  void setup_app_list(const std::weak_ptr<PluginDescriptorList> &list) {
    _gui = std::make_shared<HubUI>(list);
  }

private:
  std::shared_ptr<HubUI> _gui = std::make_shared<HubUI>();
};

#endif // HUB_UI_HPP
