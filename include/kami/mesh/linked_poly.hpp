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

#include "kami/math/barycenter.hpp"
#include "kami/math/base_types.hpp"
#include "kami/math/bounds.hpp"
#include "kami/math/edge.hpp"
#include "kami/math/hmat.hpp"
#include "kami/math/overlaps.hpp"
#include "kami/math/vertex.hpp"
#include "kami/mesh/linked_edge.hpp"
#include "kami/packing/box.hpp"
#include <memory>
#include <vector>

namespace kami {

// ==========================================================================
// Linked Facet Meshing
// ==========================================================================

class LinkedPolygon {
  typedef std::vector<std::shared_ptr<LinkedPolygon>> LinkedPool;

public:
  LinkedPolygon(int N = 3) : n(math::Vertex(0, 0, 1, 0)) {
    facets = std::vector<LinkedEdge<LinkedPolygon>>(N);
  }

  // ==========================================================================
  // Getters
  // ==========================================================================

  /**
   * @brief Get the bounds for displaying this facet
   */
  const math::Bounds getBounds(bool recursive, bool stop_on_cut = true) const;

  ulong getUID() const { return uid; }

  int getParentEdgeIndex() const { return parent_edge; }

  std::string getParentEdgeName() const { return getEdgeName(parent_edge); }

  /**
   * @brief Get the barycenter of the children of this facet and itself.
   */
  const void getBarycenter(math::Barycenter &bary, bool recursive,
                           bool stop_on_cut = true) const;

  void getChildUIDs(std::vector<ulong> &uids) const {
    uids.push_back(uid);
    for (auto &f : facets) {
      if (!f.nullMesh() && f.isOwned() && !f.hasCut())
        f.getMesh()->getChildUIDs(uids);
    }
  }

  // ==========================================================================
  // Transformations
  // ==========================================================================

  /**
   * @brief Transform this mesh facet with the given homogenous matrix
   *
   * @param mat the homogenous matrix describing the transform
   */
  void transform(const math::HMat &mat, bool recusive = false,
                 bool stop_on_cut = true);

  /**
   * @brief Compute recursively the transformation to put this face into the
   * world plane and call this function on the owned children.
   */
  void unfoldMesh(long depth, long max_depth);

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
  std::vector<ulong> linkNeighbours(LinkedPool &pool);

  /**
   * @brief Merge the facets with owned child facets with the same normal
   *
   */
  void mergeSimilar(LinkedPool &pool, std::vector<ulong> &removed);

  // ==========================================================================
  // Sclicing logic
  // ==========================================================================

  /**
   * @brief Iterate over the children to find overlapping triangles. Recursively
   * pass the overlapping info to the parent from the childs.
   * When two childs report the same overlapping (i.e. A overlapping with B and
   * B overlapping with A) the parent should cut the mesh on one of the two
   * edges and displace the splitted part farther.
   */
  overlaps::MeshOverlaps
  sliceChildren(const LinkedPool &, std::vector<packing::Box<LinkedPolygon>> &);

  // ==========================================================================
  // STL Model Unfold + SVG Export
  // ==========================================================================

  /**
   * @brief Fill the given stringstream with the serialized version of this
   * facet. This function is called recursively.
   *
   * @param stream the string stream to fill
   * @param mat the transformation matrix to apply
   * @param depth the actual depth
   * @param max_depth the maximum depth
   */
  void fillSVGString(std::stringstream &stream, const math::HMat &mat,
                     const std::string &color, int depth, int max_depth);

  /**
   * @brief Fill the given stringstream with the serialized version of this
   * facet. Project the vertex onto the two given axis and with the given color.
   * This function is *not* called recursively.
   *
   * @param stream the string stream to fill
   * @param mat the transformation matrix to apply
   */
  void fillSVGProjectString(std::stringstream &stream, const math::HMat &mat,
                            const math::Vec3 &ax1, const math::Vec3 &ax2,
                            const std::string &color);

protected:
  // ==========================================================================
  // Facet description
  // ==========================================================================
  // Facet properties
  ulong uid;                                     //< The UID if this facet
  std::vector<LinkedEdge<LinkedPolygon>> facets; //< Edges of this facet
  int parent_edge = INT8_MAX;                    //< Edge to the parent
  math::Vertex n;                                //< Normal of this facet

  // Flattening
  const math::HMat std_mat; //< Basic HMat for no transforms
  math::HMat unfold_coef;   //< HMat for transforming n to the world normal

  // ==========================================================================
  // Getters
  // ==========================================================================

  /**
   * @brief Get the edge corresponding to this number
   */
  inline LinkedEdge<LinkedPolygon> getEdge(int edge) const {
    if (edge < facets.size())
      return facets[edge];
    return facets[0];
  };

  /**
   * @brief Get the edge name
   */
  inline std::string getEdgeName(int edge) const {
    std::stringstream ss;
    ss << "f" << edge + 1 << ((edge == facets.size() - 1) ? 1 : edge + 2);
    return ss.str();
  };

  /**
   * @brief Get a pointer to the parent
   *
   * @return a pointer to the parent if exist, else nullptr
   */
  inline const LinkedPolygon *getParent() const {
    if (parent_edge < facets.size())
      return facets[parent_edge].getMesh();
    return nullptr;
  };

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
    if (edge < facets.size())
      return getEdge(edge).dir(normalized);
    if (math::Edge::colinear(n, getParentNormal())) {
      return math::Vertex{1, 0, 0};
    } else {
      math::Vertex pn = getParentNormal();
      math::Vec3 dir =
          math::Vec3{n(0), n(1), n(2)}.cross(math::Vec3{pn(0), pn(1), pn(2)});
      return math::Vertex{dir(0), dir(1), dir(2), 0};
    }
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

  // ==========================================================================
  // Linking logic
  // ==========================================================================

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
  bool hasSameEdge(LinkedPolygon *parent_facet, int edge, int &on_edge);

  // ==========================================================================
  // Sclicing logic
  // ==========================================================================

  /**
   * @brief Compute the transformation and call a recusive call on the children.
   * The children will be translated along the normal of the edge direction of
   * the given distance.
   *
   * @param edge the edge which will be translated
   */
  void sliceEdge(int edge);

  /**
   * @brief Slice the mesh from the parent edge
   *
   * @param cut_number the cut UID
   */
  void cutOnParentEdge(int cut_number);

  /**
   * @brief Return the overlaps between this facets and the others
   */
  overlaps::MeshOverlaps hasOverlaps(const LinkedPool &pool);

  // ==========================================================================
  // Debug
  // ==========================================================================
  void displayInformations(std::ostream &os) const {
    os << "\tMesh " << uid << " : " << std::endl;
    for (int i = 0; i < facets.size(); i++) {
      os << "\t  - f" << i + 1 << ((i == facets.size() - 1) ? 1 : i + 2) << " "
         << facets[i] << std::endl;
    }
  };

  friend std::ostream &operator<<(std::ostream &os, const LinkedPolygon &mesh) {
    mesh.displayInformations(os);
    os << "\t  - Normal [" << mesh.n(0) << ", " << mesh.n(1) << ", "
       << mesh.n(2) << "]" << std::endl;
    return os;
  }
};

} // namespace kami

#endif
