#ifndef MESH_TREE
#define MESH_TREE

#include "microstl.hpp"
#include "svgfigure.hpp"
#include <vector>

namespace kami {

enum EdgeName { NONE, EDGE_12, EDGE_23, EDGE_31 };
constexpr float MAX_DISTANCE{1E-1};
constexpr float MAX_DISTANCE2{MAX_DISTANCE * MAX_DISTANCE};

typedef unsigned long ulong;

/**
 * @brief Represent a facet and its link with the neighbours
 *
 */
struct MeshFacet {
  const microstl::Facet *facet = nullptr;

  ulong id;

  EdgeName parent_edge = NONE;
  bool ownF12 = false;
  const MeshFacet *f12 = nullptr;
  bool ownF23 = false;
  const MeshFacet *f23 = nullptr;
  bool ownF31 = false;
  const MeshFacet *f31 = nullptr;

  MeshFacet() {}
  MeshFacet(const microstl::Facet *_facet, ulong _id)
      : facet(_facet), id(_id) {}

  std::vector<ulong> linkNeighbours(std::vector<MeshFacet> &pool);

  bool hasSamePoint(MeshFacet *parent_facet, EdgeName edge);

  SVGFigure getChildrenPatternSVGPaths(const MeshFacet *caller) const;

  static float vertexDistance2(const microstl::Vertex *v1,
                               const microstl::Vertex *v2);
  static float vertexDistance(const microstl::Vertex *v1,
                              const microstl::Vertex *v2);
};

// Methods
struct FacetPool : std::vector<MeshFacet> {
  FacetPool(unsigned long _size)
      : std::vector<MeshFacet>(std::vector<MeshFacet>(_size)) {}
  FacetPool(const microstl::Mesh &mesh);

  void makeFacetPoolInternalLink();

  void printInformations();
};

} // namespace kami

#endif