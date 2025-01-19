#include "kami/plugins/colorbook/colorbook.hpp"
#include "kami/plugins/colorbook/colorbook_ui.hpp"

ColorBookPluginDescriptor::ColorBookPluginDescriptor()
    : InternalPluginDescriptor(PLUGIN_NAME, PLUGIN_VERSION,
                               PluginType::INTERNAL) {}

std::shared_ptr<Plugin> ColorBookPluginDescriptor::instantiate() {
  return std::make_shared<ColorBookPlugin>();
}

std::shared_ptr<BaseAppUI> ColorBookPlugin::gui() {
  return std::make_shared<ColorBookUI>();
}
