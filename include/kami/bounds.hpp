#ifndef KAMI_BOUNDS
#define KAMI_BOUNDS

#include <ostream>
namespace kami {

struct Bounds {
  double xmin, xmax;
  double ymin, ymax;

  Bounds &operator+=(const Bounds &other) {
    if (other.xmin < this->xmin)
      this->xmin = other.xmin;
    if (other.xmax > this->xmax)
      this->xmax = other.xmax;
    if (other.ymin < this->ymin)
      this->ymin = other.ymin;
    if (other.ymax > this->ymax)
      this->ymax = other.ymax;
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const Bounds &b) {
    os << "Bound (" << b.xmin << ", " << b.ymin << ", ";
    os << b.xmax << ", " << b.ymax << ")";
    return os;
  }
};
} // namespace kami

#endif