#ifndef KAMI_BOUNDS
#define KAMI_BOUNDS

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
};
} // namespace kami

#endif