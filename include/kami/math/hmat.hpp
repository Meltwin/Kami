#ifndef KAMI_MATH_HMAT
#define KAMI_MATH_HMAT

#include "kami/math/base_types.hpp"

namespace kami::math {
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
} // namespace kami::math

#endif