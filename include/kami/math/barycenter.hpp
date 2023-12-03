#ifndef KAMI_MATH_BARYCENTER
#define KAMI_MATH_BARYCENTER

#include "kami/math/vertex.hpp"

namespace kami::math {
/**
 * @brief Compute a barycenter
 */
struct Barycenter {
  Barycenter() {}

  void addVertex(const Vertex &vertex) {
    if (computed) {
      computed = false;
      buffer(0) = buffer(0) * n_points;
      buffer(1) = buffer(1) * n_points;
      buffer(2) = buffer(2) * n_points;
    }
    n_points++;
    buffer(0) = buffer(0) + vertex(0);
    buffer(1) = buffer(1) + vertex(1);
    buffer(2) = buffer(2) + vertex(2);
  }

  const Vertex &getBarycenter() {
    if (!computed) {
      buffer(0) = buffer(0) / n_points;
      buffer(1) = buffer(1) / n_points;
      buffer(2) = buffer(2) / n_points;
      computed = true;
    }
    return buffer;
  }

private:
  bool computed = false;
  Vertex buffer{0, 0, 0, 0};
  int n_points = 0;
};
} // namespace kami::math

#endif