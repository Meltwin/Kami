#include "meshTree.hpp"
#include "microstl.hpp"
#include "svgfigure.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

namespace kami {

float MeshFacet::vertexDistance2(const microstl::Vertex *v1,
                                 const microstl::Vertex *v2) {
  return (v1->x - v2->x) * (v1->x - v2->x) + (v1->y - v2->y) * (v1->y - v2->y) +
         (v1->z - v2->z) * (v1->z - v2->z);
}

float MeshFacet::vertexDistance(const microstl::Vertex *v1,
                                const microstl::Vertex *v2) {
  return std::sqrt(vertexDistance2(v1, v2));
}

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

SVGFigure MeshFacet::getChildrenPatternSVGPaths(const MeshFacet *caller) const {
  SVGFigure figure(0);

  // Get for each children facet the svg paths
  if (ownF12) {
    auto childs = f12->getChildrenPatternSVGPaths(this);
    figure.insert(figure.end(), childs.begin(), childs.end());
  }
  if (ownF23) {
    auto childs = f23->getChildrenPatternSVGPaths(this);
    figure.insert(figure.end(), childs.begin(), childs.end());
  }

  if (ownF31) {
    auto childs = f31->getChildrenPatternSVGPaths(this);
    figure.insert(figure.end(), childs.begin(), childs.end());
  }

  // Adding this node
  SVGPath path(3);
  path[0] = SVGPoint{facet->v1.x, facet->v1.y};
  path[1] = SVGPoint{facet->v2.x, facet->v2.y};
  path[2] = SVGPoint{facet->v3.x, facet->v3.y};
  figure.push_back(path);

  // TODO: Rotate everything to parent normal
  return figure;
}

FacetPool::FacetPool(const microstl::Mesh &mesh)
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
