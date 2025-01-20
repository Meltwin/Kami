#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <kami/app/ui/BaseAppUI.hpp>
#include <memory>
#include <vector>

enum class PluginType { INTERNAL, CPP, PYTHON };

struct Plugin {
  typedef std::shared_ptr<Plugin> SharedPtr;
  virtual ~Plugin() = default;
  virtual std::shared_ptr<BaseAppUI> gui() = 0;
};

struct PluginDescriptor {
  typedef std::shared_ptr<PluginDescriptor> SharedPtr;

  virtual ~PluginDescriptor() = default;
  PluginDescriptor(const char *_name, const char *_version, PluginType);

  const char *name;
  const char *version;
  const PluginType type;
};

struct InternalPluginDescriptor : PluginDescriptor {
  typedef std::shared_ptr<InternalPluginDescriptor> SharedPtr;

  InternalPluginDescriptor(const char *, const char *, PluginType);
  virtual std::shared_ptr<Plugin> instantiate() { return nullptr; }
};

typedef std::vector<Plugin::SharedPtr> PluginList;
typedef std::vector<PluginDescriptor::SharedPtr> PluginDescriptorList;

#endif // PLUGIN_HPP
