#include "kami/app/plugin/plugin.hpp"

PluginDescriptor::PluginDescriptor(const char *_name, const char *_version,
                                   PluginType _type)
    : name(_name), version(_version), type(_type) {}

InternalPluginDescriptor::InternalPluginDescriptor(const char *_name,
                                                   const char *_version,
                                                   PluginType _type)
    : PluginDescriptor(_name, _version, _type) {}
