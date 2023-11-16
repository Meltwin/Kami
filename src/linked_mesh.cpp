#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"
#include "kami/svgfigure.hpp"
#include "microstl/microstl.hpp"
#include <cmath>
#include <iostream>
#include <sstream>

namespace kami {

// ==========================================================================
// Facet math functions
// ==========================================================================

Vec4 LinkedMesh::getEdgeDirection4(EdgeName edge) const {
  Vec4 direction{0, 0, 0, 1};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  // Computing the direction vector
  direction(0) = v2.x - v1.x;
  direction(1) = v2.y - v1.y;
  direction(2) = v2.z - v1.z;

  direction.normalize();

  return direction;
}

Vec3 LinkedMesh::getEdgeDirection3(EdgeName edge) const {
  Vec3 direction{0, 0, 0};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  // Computing the direction vector
  direction(0) = v2.x - v1.x;
  direction(1) = v2.y - v1.y;
  direction(2) = v2.z - v1.z;

  direction.normalize();

  return direction;
}

Vec4 LinkedMesh::getEdgePosition(EdgeName edge) const {
  Vec4 vec{0, 0, 0, 1};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  vec(0) = (v1.x + v2.x) / 2;
  vec(1) = (v1.y + v2.y) / 2;
  vec(2) = (v1.z + v2.z) / 2;

  return vec;
}

math::HMat LinkedMesh::getHRotationMatrix() const {
  math::HMat mat;

  // Get the rotation part
  if (getParent() == nullptr) {
    return mat;
  }

  // Constructing parent frame
  auto x_axis = getEdgeDirection3(parent_edge); // Edge direction == new X axis
  auto new_n = getParentNormal3(); // Parent normal direction == new Z axis
  Vec3 y_axis =
      new_n.cross(x_axis); // Y direction is the cross product of the other two
  y_axis.normalize();

  auto old_n = getNormal3(); // Child normal direction (old Z axis)

  double theta = std::atan2(y_axis.dot(old_n), new_n.dot(old_n));

  // Set the matrix for a rotation around X
  mat(1, 1) = std::cos(theta);
  mat(1, 2) = -std::sin(theta);
  mat(2, 1) = std::sin(theta);
  mat(2, 2) = std::cos(theta);

  math::simplify(mat);

  return mat;
}

math::HMat LinkedMesh::getHTransform(EdgeName edge) const {
  math::HMat mat;

  // Prevent computation if parent doesn't exist
  if (getParent() == nullptr)
    return mat;

  // Edge direction == new X axis
  auto x_axis = getEdgeDirection3(parent_edge);
  mat.setRotXAsAxis(x_axis);

  // Parent normal direction == new Z axis
  auto z_axis = getParentNormal3();
  z_axis.normalize();
  mat.setRotZAsAxis(z_axis);

  // Y direction is the cross product of the other two
  Vec3 y_axis = z_axis.cross(x_axis);
  y_axis.normalize();
  mat.setRotYAsAxis(y_axis);

  // Translation part
  mat.setTransAsAxis(getEdgePosition(parent_edge));

  return mat;
}

PathList LinkedMesh::rotatePathList(PathList &list) const {
  PathList out(0);

  // If there's no parent, stop there
  if (parent_edge == NONE)
    return list;

  // Else rotate everything
  auto rot_mat = getHRotationMatrix();
  auto trsf_mat = getHTransform(parent_edge);
  auto inv_trsf_mat = trsf_mat.invert();
  math::HMat coeff_mat = trsf_mat * (Mat4)(rot_mat * inv_trsf_mat);

  std::cout << std::endl << "Trsf" << std::endl << trsf_mat << std::endl;
  std::cout << "Rot" << std::endl << rot_mat << std::endl;
  std::cout << "Inv" << std::endl << inv_trsf_mat << std::endl;
  std::cout << std::endl << coeff_mat << std::endl;

  for (MeshPath &path : list) {
    MeshPath out_mesh(0);
    for (ulong path_idx = 0; path_idx < path.size(); path_idx++) {
      Vec4 new_position = coeff_mat * path[path_idx];
      out_mesh.push_back(new_position);
    }
    out.push_back(out_mesh);
  }
  return out;
}

// ==========================================================================
// Facet Linking Logic
// ==========================================================================

bool LinkedMesh::hasSamePoint(LinkedMesh *parent_facet, EdgeName edge) {
  // Get the vertex we want to find
  const microstl::Vertex *v1, *v2;
  switch (edge) {
  case EDGE_12:
    v1 = &(parent_facet->facet->v1);
    v2 = &(parent_facet->facet->v2);
    break;
  case EDGE_23:
    v1 = &(parent_facet->facet->v2);
    v2 = &(parent_facet->facet->v3);
    break;
  case EDGE_31:
    v1 = &(parent_facet->facet->v3);
    v2 = &(parent_facet->facet->v1);
    break;
  case NONE:
    return false;
  }

  // Test if we have common vertex
  if (math::distance2(v1, &this->facet->v1) < MAX_DISTANCE2) {
    if (math::distance2(v2, &this->facet->v2) < MAX_DISTANCE2) {
      this->f12 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_12 : parent_edge;
      return true;
    } else if (math::distance2(v2, &this->facet->v3) < MAX_DISTANCE2) {
      this->f31 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_31 : parent_edge;
      return true;
    }
  } else if (math::distance2(v1, &this->facet->v2) < MAX_DISTANCE2) {
    if (math::distance2(v2, &this->facet->v1) < MAX_DISTANCE2) {
      this->f12 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_12 : parent_edge;
      return true;
    } else if (math::distance2(v2, &this->facet->v3) < MAX_DISTANCE2) {
      this->f23 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_23 : parent_edge;
      return true;
    }
  } else if (math::distance2(v1, &this->facet->v3) < MAX_DISTANCE2) {
    if (math::distance2(v2, &this->facet->v1) < MAX_DISTANCE2) {
      this->f31 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_31 : parent_edge;
      return true;
    } else if (math::distance2(v2, &this->facet->v2) < MAX_DISTANCE2) {
      this->f23 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_23 : parent_edge;
      return true;
    }
  }

  return false;
}

std::vector<ulong> LinkedMesh::linkNeighbours(std::vector<LinkedMesh> &pool) {
  unsigned char done = ((f12 == nullptr) ? 0 : 1) | ((f23 == nullptr) ? 0 : 2) |
                       ((f31 == nullptr) ? 0 : 4);
  std::vector<ulong> created(0);

  for (ulong i = 0; i < pool.size(); i++) {
    if (id == pool[i].id)
      continue;

    bool unlinked_facet = (pool[i].parent_edge == NONE);
    if ((f12 == nullptr) && pool[i].hasSamePoint(this, EDGE_12)) {
      if (unlinked_facet) {
        ownF12 = true;
        created.push_back(i);
      }
      f12 = &pool[i];
      done |= 1;
    } else if ((f23 == nullptr) && pool[i].hasSamePoint(this, EDGE_23)) {
      if (unlinked_facet) {
        ownF23 = true;
        created.push_back(i);
      }
      f23 = &pool[i];
      done |= 2;
    } else if ((f31 == nullptr) && pool[i].hasSamePoint(this, EDGE_31)) {
      if (unlinked_facet) {
        ownF31 = true;
        created.push_back(i);
      }
      f31 = &pool[i];
      done |= 4;
    }

    if (done == 7)
      break;
  }

  return created;
}

// ==========================================================================
// STL Model Unfold + SVG Export
// ==========================================================================

PathList LinkedMesh::getChildrenInParentPlane(int max_depth, int depth) const {
  PathList list(0);

  list.push_back(getPath());

  if (max_depth == -1 || depth < max_depth) {
    if (ownF12) {
      auto childs = f12->getChildrenInParentPlane(max_depth, depth + 1);
      list.insert(list.end(), childs.begin(), childs.end());
    }
    if (ownF23) {
      auto childs = f23->getChildrenInParentPlane(max_depth, depth + 1);
      list.insert(list.end(), childs.begin(), childs.end());
    }
    if (ownF31) {
      auto childs = f31->getChildrenInParentPlane(max_depth, depth + 1);
      list.insert(list.end(), childs.begin(), childs.end());
    }
  }

  list = rotatePathList(list);

  return list;
}

SVGFigure LinkedMesh::getChildrenPatternSVGPaths(int max_depth) const {
  SVGFigure figure(0);

  auto list = getChildrenInParentPlane(max_depth, 0);
  for (MeshPath &path : list) {
    SVGPath svg_path(0);

    for (Vec4 &point : path) {
      svg_path.push_back(SVGPoint{point(0), point(1)});
    }

    figure.push_back(svg_path);
  }

  return figure;
}

// ==========================================================================
// Facet Pool Logic
// ==========================================================================

LinkedMeshPool::LinkedMeshPool(microstl::Mesh &mesh)
    : std::vector<LinkedMesh>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++)
    (*this)[i] = LinkedMesh(&mesh.facets[i], i);
}

void LinkedMeshPool::makeFacetPoolInternalLink() {
  ulong index = 0;
  std::vector<ulong> stack{0};
  while ((index < stack.size()) && (index < this->size())) {
    auto created = (*this)[stack[index]].linkNeighbours(*this);
    stack.insert(stack.end(), created.begin(), created.end());
    index++;
  }
}

void LinkedMeshPool::printInformations() {
  int solo = 0;       // Get Number of solo facets
  int non_owning = 0; // Number of non owning facets
  for (LinkedMesh &facet : *this) {
    if (facet.f12 == nullptr && facet.f23 == nullptr && facet.f31 == nullptr)
      solo++;
    if (!facet.ownF12 && !facet.ownF23 && !facet.ownF31)
      non_owning++;
  }
  std::cout << "Pool Informations" << std::endl;
  std::cout << "\tSolo facets = " << solo << std::endl;
  std::cout << "\tNon Owning facets = " << non_owning << std::endl;
}

} // namespace kami
