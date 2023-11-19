#ifndef KAMI_MATH
#define KAMI_MATH

#include "microstl/microstl.hpp"
#include <cmath>
#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <eigen3/Eigen/src/Core/Product.h>
#include <iostream>

// ==========================================================================
// Type defining
// ==========================================================================
typedef unsigned long ulong;
typedef Eigen::Vector<double, 3> Vec3;
typedef Eigen::Vector<double, 4> Vec4;
typedef Eigen::Matrix<double, 4, 4> Mat4;

namespace kami::math {

// ==========================================================================
// Constants
// ==========================================================================

constexpr double SIMPLIFICATION_THRESHOLD(1E-7);
constexpr double MAX_DISTANCE{1E-3};
constexpr double MAX_DISTANCE2{MAX_DISTANCE * MAX_DISTANCE};

// ==========================================================================
// Matrix utils functions
// ==========================================================================

/**
 * @brief Represent an homogenous matrix for 3D transformations
 *
 */
struct HMat : public Mat4 {
  enum ROW_INDEX : int { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, TRANSL = 3 };

  HMat() : Mat4{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}} {}
  HMat(const Eigen::Product<Mat4, Mat4> &other) : Mat4(other) {}
  HMat(const Mat4 &other) : Mat4(other) {}

  /**
   * @brief Set the column value from a Vec3 vector
   *
   * @param vec3 the vector to insert
   * @param col the column index
   * @param normalize true if we should normalize the vector before
   */
  inline void setColAsAxis(Vec3 &vec3, ROW_INDEX col, bool normalize = false) {
    if (normalize)
      vec3.normalize();
    (*this)(0, col) = vec3(0);
    (*this)(1, col) = vec3(1);
    (*this)(2, col) = vec3(2);
  }

  /**
   * @brief Set the column value from a Vec4 vector
   *
   * @param vec4 the vector to insert
   * @param col the column index
   * @param normalize true if we should normalize the vector before
   */
  inline void setColAsAxis(Vec4 &vec4, int col, bool normalize = false) {
    if (normalize)
      vec4.normalize();
    (*this)(0, col) = vec4(0);
    (*this)(1, col) = vec4(1);
    (*this)(2, col) = vec4(2);
  }

  /**
   * @brief Set X rot from a vector
   */
  void setRotXAsAxis(Vec3 vec3) { setColAsAxis(vec3, X_AXIS, true); }
  /**
   * @brief Set Y rot from a vector
   */
  void setRotYAsAxis(Vec3 vec3) { setColAsAxis(vec3, Y_AXIS, true); }
  /**
   * @brief Set Z rot from a vector
   */
  void setRotZAsAxis(Vec3 vec3) { setColAsAxis(vec3, Z_AXIS, true); }
  /**
   * @brief Set X rot from a vector
   */
  void setRotXAsAxis(Vec4 vec4) { setColAsAxis(vec4, X_AXIS, true); }
  /**
   * @brief Set Y rot from a vector
   */
  void setRotYAsAxis(Vec4 vec4) { setColAsAxis(vec4, Y_AXIS, true); }
  /**
   * @brief Set Z rot from a vector
   */
  void setRotZAsAxis(Vec4 vec4) { setColAsAxis(vec4, Z_AXIS, true); }
  /**
   * @brief Set translation from a vector
   */
  void setTransAsAxis(Vec3 vec3) { setColAsAxis(vec3, TRANSL); }
  /**
   * @brief Set translation from a vector
   */
  void setTransAsAxis(Vec4 vec4) { setColAsAxis(vec4, TRANSL); }

  /**
   * @brief Invert the homogenous matrix through the direct inverse formula for
   * an homogenous matrix.
   * Inverse of rotation R = R^T
   * Inverse of translation P = -R^T * P
   *
   * @return HMat
   */
  HMat invert() {
    HMat out;

    // Invert rotation
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        out(i, j) = (*this)(j, i);

    // New translation
    for (int i = 0; i < 3; i++)
      out(i, 3) = -(out(i, 0) * (*this)(0, 3) + out(i, 1) * (*this)(1, 3) +
                    out(i, 2) * (*this)(2, 3));

    return out;
  }

  /**
   * @brief Simplify the matrix by removing too small coefficient (<
   * SIMPLIFICATION_THRESOLD)
   */
  void simplify() {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        (*this)(i, j) = (std::fabs((*this)(i, j)) < SIMPLIFICATION_THRESHOLD)
                            ? 0
                            : (*this)(i, j);
  }
};

// ==========================================================================
// Vertex utils functions
// ==========================================================================

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
  Vertex directionTo(const Vertex &other) const {
    Vertex out{other(0) - (*this)(0), other(1) - (*this)(1),
               other(2) - (*this)(2)};
    out.normalize();
    return out;
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

  operator Vec3() const { return {(*this)(0), (*this)(1), (*this)(2)}; }
};

} // namespace kami::math

#endif