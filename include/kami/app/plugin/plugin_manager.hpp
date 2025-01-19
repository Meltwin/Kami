#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include "kami/app/logger.hpp"
#include "kami/app/plugin/hub.hpp"
#include "kami/app/plugin/plugin.hpp"
#include <memory>
#include <vector>

class PluginManager {
public:
  typedef std::shared_ptr<PluginManager> SharedPtr;
  static SharedPtr init() { return std::make_shared<PluginManager>(); }

  void load_plugins() {
    load_internal_plugins();
    load_external_plugins();
  }

  std::shared_ptr<Plugin> app() { return current_application; }

private:
  void load_internal_plugins() const;
  void load_external_plugins() const;

private:
  std::shared_ptr<Hub> hub = std::make_shared<Hub>();
  Plugin::SharedPtr current_application = hub;
  std::unique_ptr<PluginDescriptorList> applications =
      std::make_unique<PluginDescriptorList>();
  PluginDescriptorList plugins;
  const Logger logger = Logger("Kami Plugins");
};

#endif // PLUGIN_MANAGER_HPP
