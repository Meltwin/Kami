#ifndef KAMI_EXPORT_PAPER_FORMAT
#define KAMI_EXPORT_PAPER_FORMAT

#include <ostream>

namespace kami::out {

struct PaperFormat {
  double width, height;

  friend std::ostream &operator<<(std::ostream &os, const PaperFormat &format) {
    os << "(" << format.width << ", " << format.height << ")";
    return os;
  }
};

// ==========================================================================
// Iso A Paper
// ==========================================================================
// A Series sizes
constexpr double A0_WIDTH{841};
constexpr double A0_HEIGHT{1189};

template <int N> struct PaperA : public PaperFormat {
  PaperA() {
    width = A0_WIDTH;
    height = A0_HEIGHT;
    for (int i = 0; i < N; i++) {
      auto new_height = width / 2;
      width = height;
      height = new_height;
    }
  }
};

} // namespace kami::out

#endif