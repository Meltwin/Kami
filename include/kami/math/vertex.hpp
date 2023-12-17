#ifndef KAMI_MATH_VERTEX
#define KAMI_MATH_VERTEX

#include "kami/math/base_types.hpp"
#include "microstl/microstl.hpp"
#include <iostream>

namespace kami::math {
/**
 * @brief Represent a mesh vertex
 *
 */
struct Vertex : public Vec4 {

  Vertex(microstl::Vertex &mvertex)
      : Vec4{mvertex.x, mvertex.y, mvertex.z, 1} {}
  Vertex(microstl::Normal &mnormal)
      : Vec4{mnormal.x, mnormal.y, mnormal.z, 1} {}
  Vertex(double x, double y, double z, double w = 1) : Vec4{x, y, z, w} {}
  Vertex(const Vec4 &other) : Vec4(other) {}

  /**
   * @brief Check whether two vertices are considered the same (close enough
   * from each other)
   *
   * @param vertex the vertex to check
   * @return true if the vertices are close enough, thus are the same
   * @return false if the vertices are too far to each other, thus are not the
   * same
   */
  inline bool sameAs(const Vertex &vertex) const {
    return (Vertex::distance2(*this, vertex) < MAX_DISTANCE2);
  }

  /**
   * @brief Construct a direction vector starting from the calling vertex to the
   * given vertex.
   *
   * @param other the destination vertex
   */
  Vertex directionTo(const Vertex &other, bool normalized = false) const {
    Vertex out{other(0) - (*this)(0), other(1) - (*this)(1),
               other(2) - (*this)(2)};
    if (normalized)
      out.normalize();
    return out;
  }

  /**
   * @brief Simplify the matrix by removing too small coefficient (<
   * SIMPLIFICATION_THRESOLD)
   */
  void simplify() {
    for (int i = 0; i < 4; i++)
      (*this)(i) =
          (std::fabs((*this)(i)) < SIMPLIFICATION_THRESHOLD) ? 0 : (*this)(i);
  }

  /**
   * @brief Compute the euclidian distance to the power of two between the two
   * vertices.
   */
  static double distance2(const Vertex &v1, const Vertex &v2) {
    return (v1(0) - v2(0)) * (v1(0) - v2(0)) +
           (v1(1) - v2(1)) * (v1(1) - v2(1)) +
           (v1(2) - v2(2)) * (v1(2) - v2(2));
  }

  /**
   * @brief Compute the euclian distance between the two vertices.
   */
  static double distance(const Vertex &v1, const Vertex &v2) {
    return std::sqrt(Vertex::distance2(v1, v2));
  }

  /**
   * @brief Compute the geometrical center of the given vector of vertices.
   *
   * @param vertices a vector containing of the vertices
   * @return A vertex representing the geometrical center of the point cloud
   */
  static Vertex barycenter(const std::vector<Vertex> &vertices) {
    Vertex bary{0, 0, 0};

    for (const Vertex &vertex : vertices)
      bary += vertex;
    bary /= vertices.size();

    return bary;
  }

  operator Vec3() const { return Vec3{(*this)(0), (*this)(1), (*this)(2)}; }
};

struct VertexPair : public std::pair<Vertex, Vertex> {
  VertexPair() : std::pair<Vertex, Vertex>(Vertex(0, 0, 0), Vertex(0, 0, 0)) {}
  VertexPair(const Vertex &v1, const Vertex &v2)
      : std::pair<Vertex, Vertex>(v1, v2) {}
};
} // namespace kami::math

#endif