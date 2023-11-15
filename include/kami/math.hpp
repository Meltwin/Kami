#ifndef KAMI_MATH
#define KAMI_MATH

#include "eigen3/Eigen/Geometry"
#include <eigen3/Eigen/src/Core/Matrix.h>

namespace kami {
typedef unsigned long ulong;
typedef Eigen::Vector<double, 3> Vec3;
typedef Eigen::Vector<double, 4> Vec4;
typedef Eigen::Matrix<double, 4, 4> HMat;

constexpr double THRESHOLD(1E-6);

inline HMat getStandardHMat() {
  HMat mat{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  for (int i = 0; i < 4; i++)
    mat(i, i) = 1;
  return mat;
}

inline void simplify(HMat &mat) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      mat(i, j) = (mat(i, j) < THRESHOLD) ? 0 : mat(i, j);
}

inline void simplify(Vec4 &vec4) {
  for (int j = 0; j < 4; j++)
    vec4(j) = (vec4(j) < THRESHOLD) ? 0 : vec4(j);
}

} // namespace kami

#endif