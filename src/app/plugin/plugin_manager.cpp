#include "kami/app/plugin/plugin_manager_impl.hpp"
#include "kami/plugins/colorbook/colorbook.hpp"

PluginManager::PluginManager() : IPluginManager() { hub->setup_app_list(this); }

PluginManager::~PluginManager() {
  current_application.reset();
  hub.reset();
  applications.resize(0);
  plugins.resize(0);
}

void PluginManager::load_internal_plugins() {
#define REGISTER_PLUGIN(desc)                                                  \
  {                                                                            \
    logger.info("Loaded plugin `%s` (v. %s)", desc->name, desc->version);      \
    applications.push_back(desc);                                              \
  }

  logger.info("Loading internal plugins");
  REGISTER_PLUGIN(ColorBookPlugin::get_descriptor());
}

void PluginManager::load_external_plugins() {
  logger.info("Loading external plugins");
}

void PluginManager::load_application(const char *app_name) {
  // Get application
  logger.debug("Looking for application %s", app_name);
  PluginDescriptor::SharedPtr app_desc = nullptr;
  for (const auto &app : applications) {
    if (const size_t len = std::min(strlen(app_name), strlen(app->name));
        strncmp(app->name, app_name, len) == 0) {
      app_desc = app;
      break;
    }
  }

  // Load application
  if (app_desc == nullptr) {
    logger.error("Application %s does not exist !", app_name);
    return;
  }
  load_application(app_desc);
}

void PluginManager::load_application(PluginDescriptor::SharedPtr app_desc) {
  if (app_desc == nullptr) {
    logger.error("Trying to load application from a nullptr descriptor");
    return;
  }
  switch (app_desc->type) {
  case PluginType::CPP:
    logger.error("External C++ application loading is not implemented yet");
    break;
  case PluginType::PYTHON:
    logger.error("External Python application loading is not implemented yet");
    break;
  case PluginType::INTERNAL:
    const auto casted_desc =
        std::static_pointer_cast<InternalPluginDescriptor>(app_desc);
    current_application = casted_desc->instantiate();
    break;
  }
}
