#include "kami/meshTree.hpp"
#include "kami/math.hpp"
#include "kami/svgfigure.hpp"
#include "microstl/microstl.hpp"
#include <iostream>
#include <sstream>

namespace kami {

// ==========================================================================
// Facet getters
// ==========================================================================

VertexPair MeshFacet::getEdgeVertex(EdgeName edge) const {
  switch (edge) {
  case EDGE_12:
    return VertexPair{facet->v1, facet->v2};
  case EDGE_23:
    return VertexPair{facet->v2, facet->v3};
  case EDGE_31:
    return VertexPair{facet->v3, facet->v1};
  default:
    return VertexPair{facet->v1, facet->v2};
  }
}

MeshPath MeshFacet::getPath() const {
  if (facet == nullptr)
    return MeshPath(0);
  return MeshPath{
      Vec4{facet->v1.x, facet->v1.y, facet->v1.z, 1},
      Vec4{facet->v2.x, facet->v2.y, facet->v2.z, 1},
      Vec4{facet->v3.x, facet->v3.y, facet->v3.z, 1},
  };
}

// ==========================================================================
// Facet math functions
// ==========================================================================

float MeshFacet::vertexDistance2(const microstl::Vertex *v1,
                                 const microstl::Vertex *v2) {
  return (v1->x - v2->x) * (v1->x - v2->x) + (v1->y - v2->y) * (v1->y - v2->y) +
         (v1->z - v2->z) * (v1->z - v2->z);
}

Vec4 MeshFacet::getNormal4() const {
  return Vec4{facet->n.x, facet->n.y, facet->n.z, 1};
}

Vec3 MeshFacet::getNormal3() const {
  return Vec3{facet->n.x, facet->n.y, facet->n.z};
}

Vec4 MeshFacet::getParentNormal4() const {
  if (getParent() != nullptr)
    return getParent()->getNormal4();
  return Vec4{0, 0, 1, 1};
}

Vec3 MeshFacet::getParentNormal3() const {
  if (getParent() != nullptr)
    return getParent()->getNormal3();
  return Vec3{0, 0, 1};
}

Vec4 MeshFacet::getEdgeDirection4(EdgeName edge) const {
  Vec4 direction{0, 0, 0, 0};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  // Computing the direction vector
  direction(0) = v2.x - v1.x;
  direction(1) = v2.y - v1.y;
  direction(2) = v2.z - v1.z;
  direction(3) = 1;

  return direction;
}

Vec3 MeshFacet::getEdgeDirection3(EdgeName edge) const {
  Vec3 direction{0, 0, 0};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  // Computing the direction vector
  direction(0) = v2.x - v1.x;
  direction(1) = v2.y - v1.y;
  direction(2) = v2.z - v1.z;

  return direction;
}

Vec4 MeshFacet::getEdgePosition(EdgeName edge) const {
  Vec4 vec{0, 0, 0, 0};

  // Getting the vertex
  auto v12 = getEdgeVertex((edge != NONE) ? edge : EDGE_12);
  auto v1 = v12.first;
  auto v2 = v12.second;

  vec(0) = (v1.x + v2.x) / 2;
  vec(1) = (v1.y + v2.y) / 2;
  vec(2) = (v1.z + v2.z) / 2;
  vec(3) = 1;

  return vec;
}

HMat MeshFacet::getHRotationMatrix() const {
  auto mat = getStandardHMat();

  // Get the rotation part
  if (getParent() == nullptr) {
    return mat;
  }
  auto parent_n = getParentNormal3();
  float dot_product = parent_n.dot(getNormal3());
  float cos = dot_product / (parent_n.norm() * getNormal3().norm());
  float theta;
  if (cos >= 1.f)
    theta = 0;
  else
    theta = -std::acos(dot_product / (parent_n.norm() * getNormal3().norm()));

  // Set the matrix for a rotation around X
  mat(1, 1) = std::cos(theta);
  mat(1, 2) = -std::sin(theta);
  mat(2, 1) = std::sin(theta);
  mat(2, 2) = std::cos(theta);

  return mat;
}

HMat MeshFacet::getHTransform(EdgeName edge) const {
  auto mat = getStandardHMat();

  // Prevent computation if parent doesn't exist
  if (getParent() == nullptr)
    return mat;

  // Edge direction == new X axis
  auto x_axis = getEdgeDirection3(parent_edge);
  x_axis.normalize();
  mat(0, 0) = x_axis(0);
  mat(1, 0) = x_axis(1);
  mat(2, 0) = x_axis(2);

  // Parent normal direction == new Z axis
  auto z_axis = getParentNormal3();
  z_axis.normalize();
  mat(0, 2) = z_axis(0);
  mat(1, 2) = z_axis(1);
  mat(2, 2) = z_axis(2);

  // Y direction is the cross product of the other two
  Vec3 y_axis = z_axis.cross(x_axis);
  y_axis.normalize();
  mat(0, 1) = y_axis(0);
  mat(1, 1) = y_axis(1);
  mat(2, 1) = y_axis(2);

  // Translation part
  auto t = getEdgePosition(parent_edge);
  mat(0, 3) = t(0);
  mat(1, 3) = t(1);
  mat(2, 3) = t(2);

  return mat;
}

PathList MeshFacet::rotatePathList(PathList &list) const {
  PathList out(0);
  // If there's no parent, stop there
  if (parent_edge == NONE)
    return list;

  // Else rotate everything
  auto rot_mat = getHRotationMatrix();
  auto trsf_mat = getHTransform(parent_edge);
  HMat inv_trsf_mat = trsf_mat.inverse();
  /*std::cout << std::endl << "Trsf" << std::endl << trsf_mat << std::endl;
  std::cout << "Rot" << std::endl << rot_mat << std::endl;
  std::cout << "Inv" << std::endl << inv_trsf_mat << std::endl;*/

  HMat coeff_mat = trsf_mat * rot_mat * inv_trsf_mat;
  // HMat coeff_mat = inv_trsf_mat * rot_mat * trsf_mat;

  std::cout << std::endl << coeff_mat << std::endl;

  for (MeshPath &path : list) {
    MeshPath out_mesh(0);
    for (ulong path_idx = 0; path_idx < path.size(); path_idx++) {
      Vec4 new_position = coeff_mat * path[path_idx];
      //  std::cout << std::endl << coeff_mat * path[path_idx] << std::endl;
      out_mesh.push_back(new_position);
    }
    out.push_back(out_mesh);
  }
  return out;
}

// ==========================================================================
// Facet Linking Logic
// ==========================================================================

bool MeshFacet::hasSamePoint(MeshFacet *parent_facet, EdgeName edge) {
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
  if (vertexDistance2(v1, &this->facet->v1) < MAX_DISTANCE2) {
    if (vertexDistance2(v2, &this->facet->v2) < MAX_DISTANCE2) {
      this->f12 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_12 : parent_edge;
      return true;
    } else if (vertexDistance2(v2, &this->facet->v3) < MAX_DISTANCE2) {
      this->f31 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_31 : parent_edge;
      return true;
    }
  } else if (vertexDistance2(v1, &this->facet->v2) < MAX_DISTANCE2) {
    if (vertexDistance2(v2, &this->facet->v1) < MAX_DISTANCE2) {
      this->f12 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_12 : parent_edge;
      return true;
    } else if (vertexDistance2(v2, &this->facet->v3) < MAX_DISTANCE2) {
      this->f23 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_23 : parent_edge;
      return true;
    }
  } else if (vertexDistance2(v1, &this->facet->v3) < MAX_DISTANCE2) {
    if (vertexDistance2(v2, &this->facet->v1) < MAX_DISTANCE2) {
      this->f31 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_31 : parent_edge;
      return true;
    } else if (vertexDistance2(v2, &this->facet->v2) < MAX_DISTANCE2) {
      this->f23 = parent_facet;
      parent_edge = (parent_edge == NONE) ? EDGE_23 : parent_edge;
      return true;
    }
  }

  return false;
}

std::vector<ulong> MeshFacet::linkNeighbours(std::vector<MeshFacet> &pool) {
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

PathList MeshFacet::getChildrenInParentPlane() const {
  PathList list(0);

  if (ownF12) {
    auto childs = f12->getChildrenInParentPlane();
    list.insert(list.end(), childs.begin(), childs.end());
  }
  if (ownF23) {
    auto childs = f23->getChildrenInParentPlane();
    list.insert(list.end(), childs.begin(), childs.end());
  }
  if (ownF31) {
    auto childs = f31->getChildrenInParentPlane();
    list.insert(list.end(), childs.begin(), childs.end());
  }
  list.push_back(getPath());
  std::cout << "\t" << id << " - " << list.size();

  ulong min = 9999, max = 0;
  for (MeshPath &path : list) {
    ulong n_points = path.size();
    if (n_points < min)
      min = n_points;
    if (n_points > max)
      max = n_points;
  }
  std::cout << " [" << min << " - " << max << "] ";

  list = rotatePathList(list);
  std::cout << "\t\t " << list.size();

  min = 9999, max = 0;
  for (MeshPath &path : list) {
    ulong n_points = path.size();
    if (n_points < min)
      min = n_points;
    if (n_points > max)
      max = n_points;
  }
  std::cout << " [" << min << " - " << max << "] " << std::endl;

  return list;
}

SVGFigure MeshFacet::getChildrenPatternSVGPaths() const {
  SVGFigure figure(0);

  auto list = getChildrenInParentPlane();
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

FacetPool::FacetPool(microstl::Mesh &mesh)
    : std::vector<MeshFacet>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++)
    (*this)[i] = MeshFacet(&mesh.facets[i], i);
}

void FacetPool::makeFacetPoolInternalLink() {
  ulong index = 0;
  std::vector<ulong> stack{0};
  while ((index < stack.size()) && (index < this->size())) {
    auto created = (*this)[stack[index]].linkNeighbours(*this);
    stack.insert(stack.end(), created.begin(), created.end());
    index++;
  }
}

void FacetPool::printInformations() {
  int solo = 0;       // Get Number of solo facets
  int non_owning = 0; // Number of non owning facets
  for (MeshFacet &facet : *this) {
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
