#ifndef MESH_TREE
#define MESH_TREE

/**
 * @file meshTree.hpp
 * @author Meltwin (meltwin.github.io)
 * @brief Mesh facets description & logic
 * @version 0.1
 * @date 2023-11-14
 *
 * @copyright Copyright (c) Meltwin 2023, under MIT licence
 */

#include "kami/math.hpp"
#include "kami/svgfigure.hpp"
#include "microstl/microstl.hpp"
#include <cmath>
#include <vector>

namespace kami {

constexpr float MAX_DISTANCE{1E-1};
constexpr float MAX_DISTANCE2{MAX_DISTANCE * MAX_DISTANCE};

enum EdgeName { NONE, EDGE_12, EDGE_23, EDGE_31 };

typedef std::vector<Vec4> MeshPath;
typedef std::vector<MeshPath> PathList;
typedef std::pair<microstl::Vertex, microstl::Vertex> VertexPair;

// ==========================================================================
// Facet description
// ==========================================================================

/**
 * @brief Represent a facet and its link with the neighbours
 *
 */
struct MeshFacet {
  microstl::Facet *facet = nullptr;

  ulong id;

  EdgeName parent_edge = NONE;
  bool ownF12 = false;
  const MeshFacet *f12 = nullptr;
  bool ownF23 = false;
  const MeshFacet *f23 = nullptr;
  bool ownF31 = false;
  const MeshFacet *f31 = nullptr;

  MeshFacet() {}
  MeshFacet(microstl::Facet *_facet, ulong _id) : facet(_facet), id(_id) {}

  // ==========================================================================
  // Getters
  // ==========================================================================

  /**
   * @brief Get a pointer to the parent
   *
   * @return a pointer to the parent if exist, else nullptr
   */
  inline const MeshFacet *getParent() const {
    switch (parent_edge) {
    case EDGE_12:
      return f12;
    case EDGE_23:
      return f23;
    case EDGE_31:
      return f31;
    default:
      return nullptr;
    }
  }

  /**
   * @brief Return the vertex defining the given edge
   *
   * @param name the edge we want the vertexes of
   */
  VertexPair getEdgeVertex(EdgeName name) const;

  /**
   * @brief Get the path for drawing this mesh
   *
   * @return a path object
   */
  MeshPath getPath() const;

  // ==========================================================================
  // Maths
  // ==========================================================================

  /**
   * @brief Get the euclidian distance between two vertex to the power of two.
   *
   * @param v1 the first vertex
   * @param v2 the second vertex
   * @return float the distance to the power of two
   */
  static float vertexDistance2(const microstl::Vertex *v1,
                               const microstl::Vertex *v2);

  /**
   * @brief Compute the euclidian distance between the two vertex
   *
   * @param v1 the first vertex
   * @param v2 the second vertex
   * @return float the distance
   */
  static inline float vertexDistance(const microstl::Vertex *v1,
                                     const microstl::Vertex *v2) {
    return std::sqrt(vertexDistance2(v1, v2));
  }

  /**
   * @brief Get the Normal vector of this facet
   *
   * @return an homogenous eigen vector
   */
  Vec4 getNormal4() const;

  /**
   * @brief Get the Normal vector of the parent of this facet. If there is no
   * parent, return the Z-axis vector.
   *
   * @return an homogenous eigen vector
   */
  Vec4 getParentNormal4() const;

  /**
   * @brief Get the Normal vector of this facet
   *
   * @return Vec3 a eigen vector
   */
  Vec3 getNormal3() const;

  /**
   * @brief Get the Normal vector of the parent of this facet. If there is no
   * parent, return the Z-axis vector.
   *
   * @return Vec3 a eigen vector
   */
  Vec3 getParentNormal3() const;

  /**
   * @brief Get the Edge directionnal vector for the given edge.
   *
   * @param edge the edge we will make the vector for
   * @return an homogenous eigen vector representing the edge direction
   */
  Vec4 getEdgeDirection4(EdgeName edge) const;

  /**
   * @brief Get the Edge directionnal vector for the given edge.
   *
   * @param edge the edge we will make the vector for
   * @return a eigen vector representing the edge direction
   */
  Vec3 getEdgeDirection3(EdgeName edge) const;

  /**
   * @brief Get the Edge position in the world.
   *
   * @param edge the edge we will make the vector for
   * @return Vec3 a eigen vector representing the edge position
   */
  Vec4 getEdgePosition(EdgeName edge) const;

  /**
   * @brief Get the rotation matrix for a transformation between the two normals
   * of this facets and its parent.
   *
   * @return HMat a 4x4 homogenous eigen matrix representing this rotation
   */
  HMat getHRotationMatrix() const;

  /**
   * @brief Compute the transformation matrix between the world and the given
   * edge on this facet.
   *
   * @return HMat a 4x4 homogenous eigen matrix representing this rotation.
   */
  HMat getHTransform(EdgeName edge) const;

  /**
   * @brief Rotate a path list around the parent edge of the current facet.
   *
   * @param list the list to rotate
   */
  PathList rotatePathList(PathList &list) const;

  // ==========================================================================
  // Linking logic
  // ==========================================================================

  /**
   * @brief Search for neighbours facets in the pool and create a link to
   * them. Get the ownership on these facets if no one is linked to them.
   *
   * @param pool the pool of facets of the STL file
   * @return std::vector<ulong> the index of the owned facets
   */
  std::vector<ulong> linkNeighbours(std::vector<MeshFacet> &pool);

  /**
   * @brief Test whether the caller has two common points with this facet.
   * If two points are found, then they are neighbours on the given edge.
   * This function link the called facet to the caller facet in the case they
   * are neighbours.
   *
   * @param parent_facet the caller facet
   * @param edge the edge that they should share
   * @return true if the two facets are neighbours
   * @return false if they are not neighbours
   */
  bool hasSamePoint(MeshFacet *parent_facet, EdgeName edge);

  // ==========================================================================
  // STL Model Unfold + SVG Export
  // ==========================================================================

  /**
   * @brief Get all the children path aligned in the plane of this facet.
   *
   * @return the list of the path in the plane.
   */
  PathList getChildrenInParentPlane() const;

  /**
   * @brief Get the Children Pattern S V G Paths object
   *
   * @return SVGFigure
   */
  SVGFigure getChildrenPatternSVGPaths() const;
};

// ==========================================================================
// Facets Pool
// ==========================================================================

/**
 * @brief Represent a pool of facet that are possibly not linked together
 *
 */
struct FacetPool : std::vector<MeshFacet> {
  FacetPool(unsigned long _size)
      : std::vector<MeshFacet>(std::vector<MeshFacet>(_size)) {}
  FacetPool(microstl::Mesh &mesh);

  /**
   * @brief Iterate over the facets to link them together.
   * Iterate over them in their order of linking (i.e. the first one to be
   * linked by a facets are the first one to be iterate on). It works with a
   * "FIFO buffer" filled with the id of the new facets linked and own by the
   * current facet.
   *
   */
  void makeFacetPoolInternalLink();

  /**
   * @brief Print useful informations about the pool. For debug purpose.
   *
   */
  void printInformations();
};

} // namespace kami

#endif
