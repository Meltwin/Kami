#ifndef KAMI_BOUNDS
#define KAMI_BOUNDS

#include <limits>
#include <ostream>

namespace kami::math {

struct Bounds {
  double xmin, xmax;
  double ymin, ymax;
  double zmin, zmax;

  Bounds()
      : xmin(std::numeric_limits<double>::max()),
        xmax(std::numeric_limits<double>::min()),
        ymin(std::numeric_limits<double>::max()),
        ymax(std::numeric_limits<double>::min()),
        zmin(std::numeric_limits<double>::max()),
        zmax(std::numeric_limits<double>::min()) {}
  Bounds(double _xmin, double _xmax, double _ymin, double _ymax, double _zmin,
         double _zmax)
      : xmin(_xmin), xmax(_xmax), ymin(_ymin), ymax(_ymax), zmin(_ymin),
        zmax(_ymax) {}

  Bounds &operator+=(const Bounds &other) {
    // X
    if (other.xmin < this->xmin)
      this->xmin = other.xmin;
    if (other.xmax > this->xmax)
      this->xmax = other.xmax;
    // Y
    if (other.ymin < this->ymin)
      this->ymin = other.ymin;
    if (other.ymax > this->ymax)
      this->ymax = other.ymax;
    // Z
    if (other.zmin < this->zmin)
      this->zmin = other.zmin;
    if (other.zmax > this->zmax)
      this->zmax = other.zmax;
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const Bounds &b) {
    os << "Bound (" << b.xmin << ", " << b.ymin << ", " << b.zmin << ", ";
    os << b.xmax << ", " << b.ymax << ", " << b.zmax << ")";
    return os;
  }
};
} // namespace kami::math

#endif