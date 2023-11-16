#ifndef KAMI_MATH
#define KAMI_MATH

#include "eigen3/Eigen/Geometry"
#include "microstl/microstl.hpp"
#include <cmath>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <eigen3/Eigen/src/Core/Product.h>

// ==========================================================================
// Type defining
// ==========================================================================
typedef unsigned long ulong;
typedef Eigen::Vector<double, 3> Vec3;
typedef Eigen::Vector<double, 4> Vec4;
typedef Eigen::Matrix<double, 4, 4> Mat4;

namespace kami::math {

constexpr double THRESHOLD(1E-7);

// ==========================================================================
// Matrix utils functions
// ==========================================================================
struct HMat : public Mat4 {
  enum ROW_INDEX : int { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, TRANSL = 3 };

  HMat() : Mat4{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}} {}

  HMat(const Eigen::Product<Mat4, Mat4> &other) : Mat4(other) {}

  inline void setColAsAxis(const Vec3 &vec3, int col) {
    (*this)(0, col) = vec3(0);
    (*this)(1, col) = vec3(1);
    (*this)(2, col) = vec3(2);
  }
  inline void setColAsAxis(const Vec4 &vec4, int col) {
    (*this)(0, col) = vec4(0);
    (*this)(1, col) = vec4(1);
    (*this)(2, col) = vec4(2);
  }

  void setRotXAsAxis(const Vec3 &vec3) { setColAsAxis(vec3, X_AXIS); }
  void setRotYAsAxis(const Vec3 &vec3) { setColAsAxis(vec3, Y_AXIS); }
  void setRotZAsAxis(const Vec3 &vec3) { setColAsAxis(vec3, Z_AXIS); }
  void setTransAsAxis(const Vec3 &vec3) { setColAsAxis(vec3, TRANSL); }
  void setRotXAsAxis(const Vec4 &vec4) { setColAsAxis(vec4, X_AXIS); }
  void setRotYAsAxis(const Vec4 &vec4) { setColAsAxis(vec4, Y_AXIS); }
  void setRotZAsAxis(const Vec4 &vec4) { setColAsAxis(vec4, Z_AXIS); }
  void setTransAsAxis(const Vec4 &vec4) { setColAsAxis(vec4, TRANSL); }

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
};

inline void simplify(HMat &mat) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      mat(i, j) = (std::fabs(mat(i, j)) < THRESHOLD) ? 0 : mat(i, j);
}

// ==========================================================================
// Distance implementations
// ==========================================================================

/**
 * @brief Get the euclidian distance between two vertex to the power of two.
 *
 * @param v1 the first vertex
 * @param v2 the second vertex
 * @return float the distance to the power of two
 */
inline double distance2(const microstl::Vertex *v1,
                        const microstl::Vertex *v2) {
  return (v1->x - v2->x) * (v1->x - v2->x) + (v1->y - v2->y) * (v1->y - v2->y) +
         (v1->z - v2->z) * (v1->z - v2->z);
}

/**
 * @brief Compute the euclidian distance between the two vertex
 *
 * @param v1 the first vertex
 * @param v2 the second vertex
 * @return float the distance
 */
inline double distance(const microstl::Vertex *v1, const microstl::Vertex *v2) {
  return std::sqrt(distance2(v1, v2));
}

} // namespace kami::math

#endif