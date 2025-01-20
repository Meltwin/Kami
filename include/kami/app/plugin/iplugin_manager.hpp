
#ifndef IPLUGIN_MANAGER_HPP
#define IPLUGIN_MANAGER_HPP

#include "kami/app/plugin/plugin.hpp"

struct IPluginManager {
  virtual ~IPluginManager() = default;
  virtual void load_application(const char *) = 0;
  virtual void load_application(PluginDescriptor::SharedPtr) = 0;
  virtual PluginDescriptorList &get_applications() = 0;
};

#endif // IPLUGIN_MANAGER_HPP
