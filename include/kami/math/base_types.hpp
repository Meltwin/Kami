#ifndef KAMI_MATH_TYPES
#define KAMI_MATH_TYPES

#include <eigen3/Eigen/Eigen>

namespace kami::math {

// ==========================================================================
// Type defining
// ==========================================================================

typedef unsigned long ulong;
typedef Eigen::Vector<double, 3> Vec3;
typedef Eigen::Vector<double, 4> Vec4;
typedef Eigen::Matrix<double, 4, 4> Mat4;

// ==========================================================================
// Constants
// ==========================================================================

constexpr double SIMPLIFICATION_THRESHOLD(1E-6);
constexpr double MAX_DISTANCE{1E-3};
constexpr double MAX_DISTANCE2{MAX_DISTANCE * MAX_DISTANCE};

} // namespace kami::math

#endif