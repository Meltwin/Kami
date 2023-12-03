#ifndef KAMI_PACKING_BOX
#define KAMI_PACKING_BOX

#include "kami/math/bounds.hpp"
#include "kami/math/edge.hpp"
#include "kami/math/vertex.hpp"

namespace kami::packing {
template <typename T> struct Box {

  Box(T *mesh_ptr, const math::Bounds &bounds)
      : root(mesh_ptr), width(bounds.xmax - bounds.xmin),
        height(bounds.ymax - bounds.ymin) {
    id = -1;
  }
  Box(const Box &other)
      : id(other.id), root(other.root), width(other.width),
        height(other.height), x(other.x), y(other.y), rotated(other.rotated) {}

  int id = -1;
  T *root = nullptr;
  double width, height;
  double x = 0, y = 0;
  bool rotated = false;

  double getWidth() const { return (rotated) ? height : width; }
  double getHeight() const { return (rotated) ? width : height; }
  math::Edge getEdge(int edge) const {
    switch (edge + 1) {
    case 2:
      return math::Edge{math::Vertex{x + getWidth(), y, 0},
                        math::Vertex{x + getWidth(), y + getHeight(), 0}};
    case 3:
      return math::Edge{math::Vertex{x, y + getHeight(), 0},
                        math::Vertex{x + getWidth(), y + getHeight(), 0}};
    case 4:
      return math::Edge{math::Vertex{x, y, 0},
                        math::Vertex{x, y + getHeight(), 0}};
    default:
      return math::Edge{math::Vertex{x, y, 0},
                        math::Vertex{x + getWidth(), y, 0}};
    }
  }

  bool isColiding(const Box<T> &other) const {
    bool isColiding = false;

    // Test if a boxe is in another
    isColiding =
        ((x <= other.x) && (x + getWidth() >= other.x + other.getWidth()) &&
         (y <= other.y) && (y + getHeight() >= other.y + other.getHeight()));
    isColiding =
        isColiding ||
        ((x >= other.x) && (x + getWidth() <= other.x + other.getWidth()) &&
         (y >= other.y) && (y + getHeight() <= other.y + other.getHeight()));

    // Test coliding edges
    for (int i = 0; (i < 4) && !isColiding; i++) {
      for (int j = 0; (j < 4) && !isColiding; j++) {
        auto params = math::Edge::findIntersect(getEdge(i), other.getEdge(j));
        isColiding = ((math::Edge::VERTEX_AREA < params.t &&
                       params.t < 1 - math::Edge::VERTEX_AREA) &&
                      (math::Edge::VERTEX_AREA < params.s &&
                       params.s < 1 - math::Edge::VERTEX_AREA));
      }
    }
    return isColiding;
  }

  friend std::ostream &operator<<(std::ostream &os, Box &box) {
    os << ((box.rotated) ? " R" : "");
    os << " Box " << box.id << " (" << box.x << ", " << box.y << ", ";
    os << box.getWidth() << ", " << box.getHeight() << ") ";
    return os;
  }
};
} // namespace kami::packing

#endif