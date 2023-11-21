#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"
#include "microstl/microstl.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>

namespace kami {

// ==========================================================================
// Linked Mesh constructors
// ==========================================================================

ILinkedMesh::ILinkedMesh() : n(math::Vertex(0, 0, 0)) {}

// ==========================================================================
// (Interface) Facet math functions
// ==========================================================================

math::HMat ILinkedMesh::getHRotationMatrix() const {
  math::HMat mat;

  // Constructing parent frame
  Vec3 x_axis =
      getEdgeDirection(parent_edge, true); // Edge direction == new X axis
  Vec3 new_n = getParentNormal(); // Parent normal direction == new Z axis
  Vec3 y_axis =
      new_n.cross(x_axis); // Y direction is the cross product of the other two
  y_axis.normalize();

  Vec3 old_n = getNormal(); // Child normal direction (old Z axis)

  double theta = M_PI / 2 - std::atan2(new_n.dot(old_n), y_axis.dot(old_n));

  // Set the matrix for a rotation around X
  mat(1, 1) = std::cos(theta);
  mat(1, 2) = -std::sin(theta);
  mat(2, 1) = std::sin(theta);
  mat(2, 2) = std::cos(theta);

  mat.simplify();

  return mat;
}

math::HMat ILinkedMesh::getHTransform(int edge) const {
  math::HMat mat;

  // Edge direction == new X axis
  Vec3 x_axis = getEdgeDirection(parent_edge, true);
  mat.setRotXAsAxis(x_axis);

  // Parent normal direction == new Z axis
  Vec3 z_axis = getParentNormal();
  mat.setRotZAsAxis(z_axis);

  // Y direction is the cross product of the other two
  mat.setRotYAsAxis(z_axis.cross(x_axis));

  // Translation part
  mat.setTransAsAxis(getEdgePosition(parent_edge));

  return mat;
}

} // namespace kami
