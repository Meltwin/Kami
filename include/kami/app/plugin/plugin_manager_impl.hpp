#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include "kami/app/logger.hpp"
#include "kami/app/plugin/hub.hpp"
#include "kami/app/plugin/iplugin_manager.hpp"
#include "kami/app/plugin/plugin.hpp"
#include <memory>
#include <vector>

class PluginManager : public IPluginManager,
                      public std::enable_shared_from_this<PluginManager> {
public:
  typedef std::shared_ptr<PluginManager> SharedPtr;
  static SharedPtr init() { return std::make_shared<PluginManager>(); }

  PluginManager();
  ~PluginManager();

  void load_plugins() {
    load_internal_plugins();
    load_external_plugins();
  }

  std::shared_ptr<Plugin> app() { return current_application; }

  void load_application(const char *) override;
  void load_application(PluginDescriptor::SharedPtr) override;
  PluginDescriptorList &get_applications() override { return applications; }

private:
  void load_internal_plugins();
  void load_external_plugins();

private:
  std::shared_ptr<Hub> hub = std::make_shared<Hub>();
  Plugin::SharedPtr current_application = hub;
  PluginDescriptorList applications = PluginDescriptorList();
  PluginDescriptorList plugins;
  const Logger logger = Logger("Kami Plugins");
};

#endif // PLUGIN_MANAGER_HPP
