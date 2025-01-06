#include "kami/math/edge.hpp"
#include "kami/export/out_settings.hpp"
#include "kami/math/bounds.hpp"

namespace kami::math {

Bounds Edge::getBounds() const {
  auto b = Bounds(std::min(v1(0), v2(0)), std::max(v1(0), v2(0)),
                  std::min(v1(1), v2(1)), std::max(v1(1), v2(1)),
                  std::min(v1(2), v2(2)), std::min(v1(2), v2(2)));

  // Adding padding
  b.xmin += ((b.xmin < 0) ? 1 : -1) * out::BOUNDS_PADDING * b.xmin;
  b.xmax += ((b.xmax < 0) ? -1 : 1) * out::BOUNDS_PADDING * b.xmax;
  b.ymin += ((b.ymin < 0) ? 1 : -1) * out::BOUNDS_PADDING * b.ymin;
  b.ymax += ((b.ymax < 0) ? -1 : 1) * out::BOUNDS_PADDING * b.ymax;
  b.zmin += ((b.zmin < 0) ? 1 : -1) * out::BOUNDS_PADDING * b.zmin;
  b.zmax += ((b.zmax < 0) ? -1 : 1) * out::BOUNDS_PADDING * b.zmax;
  return b;
}

double Edge::overlapsLength(const Edge &e1, const Edge &e2) {
  // If the edges are on the same x axis
  if ((e1.v1(0) == e2.v1(0)) && (e1.v2(0) == e2.v2(0))) {
    double start =
        std::max(std::min(e1.v1(1), e1.v2(1)), std::min(e2.v1(1), e2.v2(1)));
    double end =
        std::min(std::max(e1.v1(1), e1.v2(1)), std::max(e2.v1(1), e2.v2(1)));
    return std::fabs(end - start);
  }
  // If the edges are on the same y axis
  else if ((e1.v1(1) == e2.v1(1)) && (e1.v2(1) == e2.v2(1))) {
    double start =
        std::max(std::min(e1.v1(0), e1.v2(0)), std::min(e2.v1(0), e2.v2(0)));
    double end =
        std::min(std::max(e1.v1(0), e1.v2(0)), std::max(e2.v1(0), e2.v2(0)));
    return std::fabs(end - start);
  }
  return 0;
}

IntersectParams Edge::findIntersect(const Edge &e1, const Edge &e2) {
  auto u = e1.dir();
  auto v = e2.dir();
  double det = u(1) * v(0) - u(0) * v(1);
  if (std::fabs(det) < 1E-2)
    return IntersectParams{-1, -1};

  auto dx = (e2.v1(0) - e1.v1(0)) / det;
  auto dy = (e2.v1(1) - e1.v1(1)) / det;
  return IntersectParams{dy * v(0) - dx * v(1), dy * u(0) - dx * u(1)};
}

} // namespace kami::math