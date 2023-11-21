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

#include "kami/bounds.hpp"
#include "kami/display_settings.hpp"
#include "kami/linked_edge.hpp"
#include "kami/math.hpp"
#include "kami/overlaps.hpp"
#include "microstl/microstl.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

namespace kami {

constexpr long NO_REC_LIMIT{-1};

// ==========================================================================
// Linked Facet Meshing
// ==========================================================================

struct ILinkedMesh {

  typedef std::vector<std::shared_ptr<ILinkedMesh>> LinkedPool;

  // ==========================================================================
  // Facet description
  // ==========================================================================
  // Facet properties
  ulong uid; //< The UID if this facet

  // Linking
  int parent_edge = INT8_MAX; //< Edge to the parent
  math::Vertex n;             //< Normal of this facet

  // Flattening
  const math::HMat std_mat; //< Basic HMat for no transforms
  math::HMat unfold_coef;   //< HMat for transforming n to the world normal

  ILinkedMesh();

  // ==========================================================================
  // Getters
  // ==========================================================================

  /**
   * @brief Get the edge corresponding to this number
   */
  virtual LinkedEdge<ILinkedMesh> getEdge(int edge) const = 0;

  /**
   * @brief Get the edge name
   */
  virtual std::string getEdgeName(int edge) const = 0;

  /**
   * @brief Get a pointer to the parent
   *
   * @return a pointer to the parent if exist, else nullptr
   */
  virtual const ILinkedMesh *getParent() const = 0;

  /**
   * @brief Get the bounds for displaying this facet
   */
  virtual const Bounds getBounds() const = 0;

  // ==========================================================================
  // Vertex utils
  // ==========================================================================

  /**
   * @brief Return the vertex defining the given edge
   *
   * @param name the edge we want the vertices of
   */
  math::VertexPair getEdgeVertex(int edge) const {
    return getEdge(edge).pair();
  };

  /**
   * @brief Get the Normal vector of this facet
   *
   * @return an homogenous eigen vector
   */
  inline math::Vertex getNormal() const { return n; }

  /**
   * @brief Get the Edge directionnal vector for the given edge.
   *
   * @param edge the edge we will make the vector for
   * @return an homogenous eigen vector representing the edge direction
   */
  math::Vertex getEdgeDirection(int edge, bool normalized = false) const {
    return getEdge(edge).dir(normalized);
  };

  /**
   * @brief Get the Edge position in the world.
   *
   * @param edge the edge we will make the vector for
   * @return Vec3 a eigen vector representing the edge position
   */
  math::Vertex getEdgePosition(int edge) const { return getEdge(edge).pos(); };

  /**
   * @brief Get the Normal vector of the parent of this facet. If there is no
   * parent, return the Z-axis vector.
   *
   * @return an homogenous eigen vector
   */
  math::Vertex getParentNormal() const {
    if (getParent() != nullptr)
      return getParent()->getNormal();
    return math::Vertex{0, 0, 1, 0};
  }

  /**
   * @brief Get the parent transformation matrix (transforming the parent to the
   * X-Y world plane)
   *
   * @return const math::HMat
   */
  const math::HMat getParentTrsf() const {
    if (getParent() != nullptr)
      return getParent()->unfold_coef;
    return std_mat;
  }

  // ==========================================================================
  // Transformations
  // ==========================================================================

  /**
   * @brief Get the rotation matrix for a transformation between the two
   * normals of this facets and its parent.
   *
   * @return HMat a 4x4 homogenous eigen matrix representing this rotation
   */
  math::HMat getHRotationMatrix() const;

  /**
   * @brief Compute the transformation matrix between the world and the given
   * edge on this facet.
   *
   * @return HMat a 4x4 homogenous eigen matrix representing this rotation.
   */
  math::HMat getHTransform(int edge) const;

  /**
   * @brief Transform this mesh facet with the given homogenous matrix
   *
   * @param mat the homogenous matrix describing the transform
   */
  virtual void transform(const math::HMat &mat, bool recusive = false,
                         SVGLineWidth style = SVGLineWidth::NONE) = 0;

  /**
   * @brief Compute recursively the transformation to put this face into the
   * world plane and call this function on the owned children.
   */
  virtual void unfoldMesh(long depth, long max_depth) = 0;

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
  virtual std::vector<ulong> linkNeighbours(LinkedPool &pool) = 0;

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
  virtual bool hasSameEdge(ILinkedMesh *parent_facet, int edge) = 0;

  // ==========================================================================
  // Sclicing logic
  // ==========================================================================

  /**
   * @brief Compute the transformation and call a recusive call on the children.
   * The children will be translated along the normal of the edge direction of
   * the given distance.
   *
   * @param edge the edge which will be translated
   * @param dist the distance that the edge will translated
   */
  virtual void translateChildren(int edge, double dist) = 0;

  /**
   * @brief Return the overlaps between this facets and the others
   */
  virtual overlaps::MeshOverlaps hasOverlaps(const LinkedPool &pool) = 0;

  /**
   * @brief Iterate over the children to find overlapping triangles. Recursively
   * pass the overlapping info to the parent from the childs.
   * When two childs report the same overlapping (i.e. A overlapping with B and
   * B overlapping with A) the parent should cut the mesh on one of the two
   * edges and displace the splitted part farther.
   */
  virtual overlaps::MeshOverlaps sliceChildren(const LinkedPool &pool) = 0;

  // ==========================================================================
  // STL Model Unfold + SVG Export
  // ==========================================================================

  /**
   * @brief Fill the given stringstream with the serialized version of this
   * facet. This function is called recursively.
   *
   * @param stream the string stream to fill
   * @param depth the actual depth
   * @param max_depth the maximum depth
   * @param scale_factor the factor to dilate the output
   */
  virtual void fillSVGString(std::stringstream &stream, int depth,
                             int max_depth, double scale_factor) const = 0;

  // ==========================================================================
  // Debug
  // ==========================================================================
  virtual void displayInformations(std::ostream &os) const = 0;

  friend std::ostream &operator<<(std::ostream &os, const ILinkedMesh &mesh) {
    os << "Mesh " << mesh.uid << " : " << std::endl;
    mesh.displayInformations(os);
    os << "  - Normal [" << mesh.n(0) << ", " << mesh.n(1) << ", " << mesh.n(2)
       << "]" << std::endl;
    return os;
  }
};

} // namespace kami

#endif
