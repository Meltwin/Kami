#ifndef KAMI_LINKED_EDGE
#define KAMI_LINKED_EDGE

#include "kami/bounds.hpp"
#include "kami/display_settings.hpp"
#include "kami/math.hpp"
#include <cmath>
#include <complex>
#include <sstream>

namespace kami {

constexpr double BOUNDS_PADDING{0.2f};

// ==========================================================================
// Mesh edge
// ==========================================================================

/**
 * @brief Represent an edge of a polygon
 *
 * @tparam T the mesh type
 */
template <typename T> struct LinkedEdge {

  // ==========================================================================
  // Edge description
  // ==========================================================================
  // Linking properties
  bool owned = false;
  bool cutted = false;
  T *mesh = nullptr;

  // Vertices
  math::Vertex v1;
  math::Vertex v2;

  // Style
  SVGLineWidth linestyle = SVGLineWidth::PERIMETER;

  LinkedEdge(microstl::Vertex _v1, microstl::Vertex _v2) : v1(_v1), v2(_v2) {}
  LinkedEdge() : v1(math::Vertex(0, 0, 0)), v2(math::Vertex(0, 0, 0)) {}

  // ==========================================================================
  // Utils for vertex computations
  // ==========================================================================

  /**
   * @brief Return a pair of the vertices defining this edge
   */
  inline math::VertexPair pair() const { return math::VertexPair{v1, v2}; }

  /**
   * @brief Return a vector following the direction of the edge
   */
  inline math::Vertex dir(bool normalized = false) const {
    return v1.directionTo(v2, normalized);
  }

  /**
   * @brief Return the center of the edge
   */
  inline math::Vertex pos() const { return math::Vertex::barycenter({v1, v2}); }

  /**
   * @brief Test whether the provided edge is equal to this edge.
   *
   * @param pair a pair of vertices defining the edge
   * @return true if the edges correspond
   * @return false if the edges are not the same
   */
  inline bool sameAs(const math::VertexPair &pair) const {
    return ((v1.sameAs(pair.first) && v2.sameAs(pair.second)) ||
            (v1.sameAs(pair.second) && v2.sameAs(pair.first)));
  }

  // ==========================================================================
  // Utils for slicing
  // ==========================================================================

  /**
   * @brief Find the intersection between two edges. The method used here is
   * finding the parameters for both direction vector of the edges. The
   * intersection P is then at P = s*v1.dir() = t*v2.dir(). If the vectors are
   * colinear, the function return (-1, -1).
   *
   * @param v1 the first edge
   * @param v2 the second edge
   * @return (s, t) = std::pair<double, double> the parameters for both vectors
   */
  static std::pair<double, double> findIntersect(LinkedEdge<T> &v1,
                                                 LinkedEdge<T> &v2) {
    auto u = v1.dir();
    auto v = v2.dir();
    double det = u(1) * v(0) - u(0) * v(1);
    if (std::fabs(det) < 1E-2)
      return std::pair<double, double>{-1, -1};

    auto dx = (v2.v1(0) - v1.v1(0)) / det;
    auto dy = (v2.v1(1) - v1.v1(1)) / det;
    return std::pair<double, double>{dy * v(0) - dx * v(1),
                                     dy * u(0) - dx * u(1)};
  }

  // ==========================================================================
  // SVG Export
  // ==========================================================================

  /**
   * @brief Get the bounds needed to display this edge
   */
  Bounds getBounds() const {
    auto b = Bounds{
        std::min(v1(0), v2(0)),
        std::max(v1(0), v2(0)),
        std::min(v1(1), v2(1)),
        std::max(v1(1), v2(1)),
    };

    // Adding padding
    b.xmin += ((b.xmin < 0) ? 1 : -1) * BOUNDS_PADDING * b.xmin;
    b.xmax += ((b.xmax < 0) ? -1 : 1) * BOUNDS_PADDING * b.xmax;
    b.ymin += ((b.ymin < 0) ? 1 : -1) * BOUNDS_PADDING * b.ymin;
    b.ymax += ((b.ymax < 0) ? -1 : 1) * BOUNDS_PADDING * b.ymax;
    return b;
  }

  void getAsSVGLine(std::stringstream &stream) const {
    stream << "<line ";
    stream << "x1=\"" << v1(0) << "\" ";
    stream << "y1=\"" << v1(1) << "\" ";
    stream << "x2=\"" << v2(0) << "\" ";
    stream << "y2=\"" << v2(1) << "\" ";
    appendLineStyle(linestyle, stream);
    stream << "/>\n";
  }

  // ==========================================================================
  // Debug for edge
  // ==========================================================================
  friend std::ostream &operator<<(std::ostream &os, const LinkedEdge &edge) {
    os << "Edge 1:[" << edge.v1(0) << ", " << edge.v1(1) << ", " << edge.v1(2)
       << "], 2:[" << edge.v2(0) << ", " << edge.v2(1) << ", " << edge.v2(2)
       << "]";
    if (edge.mesh != nullptr) {
      os << " ->" << ((edge.owned) ? " OWNING" : "") << " Mesh "
         << edge.mesh->uid << " on its "
         << edge.mesh->getEdgeName(edge.mesh->parent_edge) << " edge";
    }

    return os;
  }
};
} // namespace kami

#endif