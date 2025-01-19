#ifndef COLOR_BOOK_HPP
#define COLOR_BOOK_HPP

#include "kami/app/plugin/plugin.hpp"
#include <kami/common.hpp>

struct ColorBookPluginDescriptor final : InternalPluginDescriptor {
  static constchar PLUGIN_NAME{"ColorBook"};
  static constchar PLUGIN_VERSION{"1.0.0-alpha1"};

  ColorBookPluginDescriptor();
  std::shared_ptr<Plugin> instantiate() override;
};

struct ColorBookPlugin : Plugin {

  std::shared_ptr<BaseAppUI> gui() override;

  static std::shared_ptr<InternalPluginDescriptor> get_descriptor() {
    return std::make_shared<ColorBookPluginDescriptor>();
  }
};

#endif // COLOR_BOOK_HPP
