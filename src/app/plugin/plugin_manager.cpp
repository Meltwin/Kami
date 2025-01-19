#include "kami/app/plugin/plugin_manager.hpp"

#include "kami/plugins/colorbook/colorbook.hpp"

void PluginManager::load_internal_plugins() const {
#define REGISTER_PLUGIN(desc)                                                  \
  {                                                                            \
    logger.info("Loaded plugin `%s` (v. %s)", desc->name, desc->version);      \
    applications->push_back(desc);                                             \
  }

  logger.info("Loading internal plugins");
  REGISTER_PLUGIN(ColorBookPlugin::get_descriptor());
}

void PluginManager::load_external_plugins() const {
  logger.info("Loading external plugins");
}
