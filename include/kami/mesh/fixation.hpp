#ifndef KAMI_MESH_FIXATION
#define KAMI_MESH_FIXATION

namespace kami::fix {

struct FixationParameters {
  double clip_limits = 0.1;
  double height = 5;
};

} // namespace kami::fix

#endif