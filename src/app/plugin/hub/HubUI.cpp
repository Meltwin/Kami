#include "kami/app/plugin/hub.hpp"

void HubUI::MM_render_file() {}
void HubUI::MM_render_edit() {}
void HubUI::MA_custom_render() {
  ImGui::Text("Hello I'm the hub");

  if (plugin_manager == nullptr)
    return;

  for (const auto &desc : plugin_manager->get_applications()) {
    if (ImGui::Button(desc->name)) {
      plugin_manager->load_application(desc);
    }
  }
}
void HubUI::SA_custom_render() {}
