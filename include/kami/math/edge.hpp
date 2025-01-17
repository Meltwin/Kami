#ifndef KAMI_MATH_EDGE
#define KAMI_MATH_EDGE

#include "kami/math/bounds.hpp"
#include "kami/math/hmat.hpp"
#include "kami/math/vertex.hpp"

namespace kami::math {

struct IntersectParams {
  double t, s;
};

struct Edge {
public:
  static constexpr float VERTEX_AREA{1E-3};

  Edge(const Vertex &_v1, const Vertex &_v2) : v1(_v1), v2(_v2) {}
  Edge() : v1(Vertex(0, 0, 0)), v2(Vertex(0, 0, 0)) {}

  const Vertex &getFirst() const { return v1; }
  const Vertex &getSecond() const { return v2; }

  void setFirst(const Vertex &_v1) { v1 = _v1; }
  void setSecond(const Vertex &_v2) { v2 = _v2; }

  // ==========================================================================
  // Utils for vertex computations
  // ==========================================================================

  /**
   * @brief Return a vector following the direction of the edge
   */
  inline Vertex dir(bool normalized = false) const {
    return v1.directionTo(v2, normalized);
  }

  /**
   * @brief Return a pair of the vertices defining this edge
   */
  inline VertexPair pair() const { return VertexPair{v1, v2}; }

  /**
   * @brief Return the center of the edge
   */
  inline Vertex pos() const { return v1; }

  // ==========================================================================
  // Utils for edges computations
  // ==========================================================================

  /**
   * @brief Test whether the provided edge is equal to this edge.
   *
   * @param pair a pair of vertices defining the edge
   * @return true if the edges correspond
   * @return false if the edges are not the same
   */
  inline bool sameAs(const VertexPair &pair) const {
    return ((v1.sameAs(pair.first) && v2.sameAs(pair.second)) ||
            (v1.sameAs(pair.second) && v2.sameAs(pair.first)));
  }

  /**
   * @brief Get the bounds needed to display this edge
   */
  Bounds getBounds() const;

  void transformEdge(const math::HMat &mat) {
    v1 = (math::Vec4)(mat * v1);
    v2 = (math::Vec4)(mat * v2);
  }

  // ==========================================================================
  // Utils for exporting
  // ==========================================================================

  /**
   * @brief Compute on how much distance the two egdes are overlapping. Consider
   * that the edges are horizontal or vertical only.
   *
   * @param e1 the first edge
   * @param e2 the second edge
   * @return double the distance of overlapping
   */
  static double overlapsLength(const Edge &e1, const Edge &e2);

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
  static IntersectParams findIntersect(const Edge &e1, const Edge &e2);

  static bool colinear(const Vertex &v1, const Vertex &v2) {
    return (std::fabs(v1(1) * v2(2) - v1(2) * v2(1)) <= MAX_DISTANCE) &&
           (std::fabs(v1(2) * v2(0) - v1(0) * v2(2)) <= MAX_DISTANCE) &&
           (std::fabs(v1(0) * v2(1) - v1(1) * v2(0)) <= MAX_DISTANCE);
  }

  static bool sameDir(const Vertex &v1, const Vertex &v2) {
    return colinear(v1, v2) &&
           (v1(0) * v2(0) + v1(1) * v2(1) + v1(2) * v2(2) >= 0);
  }

protected:
  Vertex v1, v2;
};
} // namespace kami::math

#endif