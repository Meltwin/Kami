#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"
#include "kami/svgfigure.hpp"
#include "microstl/microstl.hpp"
#include <cmath>
#include <iostream>
#include <sstream>

namespace kami {

LinkedMesh::LinkedMesh(microstl::Facet *_facet, ulong _id)
    : f12{_facet->v1, _facet->v2}, f23(_facet->v2, _facet->v3),
      f31(_facet->v3, _facet->v1), n(_facet->n), id(_id) {
  n.normalize();
}

LinkedMesh::LinkedMesh()
    : n(math::Vertex(0, 0, 0)), f12(LinkedEdge()), f23(LinkedEdge()),
      f31(LinkedEdge()) {}

// ==========================================================================
// Facet math functions
// ==========================================================================

math::HMat LinkedMesh::getHRotationMatrix() const {
  math::HMat mat;

  // Constructing parent frame
  Vec3 x_axis = getEdgeDirection(parent_edge); // Edge direction == new X axis
  Vec3 new_n = getParentNormal(); // Parent normal direction == new Z axis
  Vec3 y_axis =
      new_n.cross(x_axis); // Y direction is the cross product of the other two
  y_axis.normalize();

  Vec3 old_n = getNormal(); // Child normal direction (old Z axis)

  double theta = std::atan2(y_axis.dot(old_n), new_n.dot(old_n));

  // Set the matrix for a rotation around X
  mat(1, 1) = std::cos(theta);
  mat(1, 2) = -std::sin(theta);
  mat(2, 1) = std::sin(theta);
  mat(2, 2) = std::cos(theta);

  mat.simplify();

  return mat;
}

math::HMat LinkedMesh::getHTransform(EdgeName edge) const {
  math::HMat mat;

  // Edge direction == new X axis
  Vec3 x_axis = getEdgeDirection(parent_edge);
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

void LinkedMesh::unfoldMesh(ulong depth, ulong max_depth) {
  // If max depth, stop
  if (max_depth != -1 && depth >= max_depth)
    return;

  // Get the cumulated transformation
  auto rot_mat = getHRotationMatrix();
  auto trsf_mat = getHTransform(parent_edge);
  auto inv_trsf_mat = trsf_mat.invert();
  flattening_coef = (Mat4)(getParentTrsf() * trsf_mat * rot_mat * inv_trsf_mat);

  std::cout << id << std::endl << flattening_coef << std::endl;

  // Rotate this face and normal
  rotate(flattening_coef);
  n = getParentNormal();

  // Rotate children
  if (f12.owned) {
    f12.mesh->unfoldMesh(depth + 1, max_depth);
  }
  if (f23.owned) {
    f23.mesh->unfoldMesh(depth + 1, max_depth);
  }
  if (f31.owned) {
    f31.mesh->unfoldMesh(depth + 1, max_depth);
  }
}

// ==========================================================================
// Facet Linking Logic
// ==========================================================================

bool LinkedMesh::hasSameEdge(LinkedMesh *parent_facet, EdgeName edge) {
  // Get the vertex we want to find
  auto pair = parent_facet->getEdgeVertex(edge);

  // Test if we have common vertex
  if (f12.sameAs(pair)) {
    f12.mesh = parent_facet;
    parent_edge = (parent_edge == NONE) ? EDGE_12 : parent_edge;
    return true;
  } else if (f23.sameAs(pair)) {
    f23.mesh = parent_facet;
    parent_edge = (parent_edge == NONE) ? EDGE_23 : parent_edge;
    return true;
  } else if (f31.sameAs(pair)) {
    f31.mesh = parent_facet;
    parent_edge = (parent_edge == NONE) ? EDGE_31 : parent_edge;
    return true;
  }

  return false;
}

std::vector<ulong> LinkedMesh::linkNeighbours(std::vector<LinkedMesh> &pool) {
  unsigned char done = ((f12.mesh == nullptr) ? 0 : 1) |
                       ((f23.mesh == nullptr) ? 0 : 2) |
                       ((f31.mesh == nullptr) ? 0 : 4);
  std::vector<ulong> created(0);

  for (ulong i = 0; i < pool.size(); i++) {
    if (id == pool[i].id)
      continue;

    bool unlinked_facet = (pool[i].parent_edge == NONE);
    if ((f12.mesh == nullptr) && pool[i].hasSameEdge(this, EDGE_12)) {
      if (unlinked_facet) {
        f12.owned = true;
        created.push_back(i);
      }
      f12.mesh = &pool[i];
      done |= 1;
    } else if ((f23.mesh == nullptr) && pool[i].hasSameEdge(this, EDGE_23)) {
      if (unlinked_facet) {
        f23.owned = true;
        created.push_back(i);
      }
      f23.mesh = &pool[i];
      done |= 2;
    } else if ((f31.mesh == nullptr) && pool[i].hasSameEdge(this, EDGE_31)) {
      if (unlinked_facet) {
        f31.owned = true;
        created.push_back(i);
      }
      f31.mesh = &pool[i];
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

SVGPath LinkedMesh::getSVGPath() const { return SVGPath({f12, f23, f31}); }

void LinkedMesh::getChildrenPatternSVGPaths(SVGFigure &figure, int depth,
                                            int max_depth) const {
  if (max_depth != -1 && depth >= max_depth)
    return;

  std::cout << "Mesh " << id << std::endl;
  std::cout << "\tF12 : " << f12 << std::endl;
  std::cout << "\tF23 : " << f23 << std::endl;
  std::cout << "\tF31 : " << f31 << std::endl;

  figure.push_back(getSVGPath());
  if (f12.owned)
    f12.mesh->getChildrenPatternSVGPaths(figure, depth + 1, max_depth);
  if (f23.owned)
    f23.mesh->getChildrenPatternSVGPaths(figure, depth + 1, max_depth);
  if (f31.owned)
    f31.mesh->getChildrenPatternSVGPaths(figure, depth + 1, max_depth);
}

// ==========================================================================
// Facet Pool Logic
// ==========================================================================

LinkedMeshPool::LinkedMeshPool(microstl::Mesh &mesh)
    : std::vector<LinkedMesh>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++) {
    (*this)[i] = LinkedMesh(&mesh.facets[i], i);
  }
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
    if (facet.f12.mesh == nullptr && facet.f23.mesh == nullptr &&
        facet.f31.mesh == nullptr)
      solo++;
    if (!facet.f12.owned && !facet.f23.owned && !facet.f31.owned)
      non_owning++;
  }
  std::cout << "Pool Informations" << std::endl;
  std::cout << "\tSolo facets = " << solo << std::endl;
  std::cout << "\tNon Owning facets = " << non_owning << std::endl;
}

} // namespace kami
