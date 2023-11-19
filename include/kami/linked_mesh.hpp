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
#include <ostream>
#include <vector>

namespace kami {

struct UnfoldingData {
  int max_depth = -1;
  int depth = 0;
};

enum EdgeName { NONE, EDGE_12, EDGE_23, EDGE_31 };
inline std::string edgeName(EdgeName edge) {
  switch (edge) {
  case NONE:
    return "NONE";
  case EDGE_12:
    return "E12";
  case EDGE_23:
    return "E23";
  case EDGE_31:
    return "E31";
  }
}
typedef std::pair<math::Vertex, math::Vertex> VertexPair;

/**
 * @brief Represent a facet and its link with the neighbours
 *
 */
struct LinkedMesh {

  // ==========================================================================
  // Mesh edge
  // ==========================================================================
  struct LinkedEdge {
    // ==========================================================================
    // Edge description
    // ==========================================================================
    // Linking properties
    bool owned = false;
    LinkedMesh *mesh = nullptr;

    // Vertexes
    math::Vertex v1;
    math::Vertex v2;

    // Style
    SVGLineWidth linewidth = SVGLineWidth::PERIMETER;

    LinkedEdge(microstl::Vertex _v1, microstl::Vertex _v2) : v1(_v1), v2(_v2) {}
    LinkedEdge() : v1(math::Vertex(0, 0, 0)), v2(math::Vertex(0, 0, 0)) {}

    // ==========================================================================
    // Utils for vertex computations
    // ==========================================================================
    inline VertexPair pair() const { return VertexPair{v1, v2}; }
    inline math::Vertex dir() const { return v1.directionTo(v2); }
    inline math::Vertex pos() const {
      return math::Vertex::barycenter({v1, v2});
    }

    inline bool sameAs(const VertexPair &pair) {
      return ((v1.sameAs(pair.first) && v2.sameAs(pair.second)) ||
              (v1.sameAs(pair.second) && v2.sameAs(pair.first)));
    }

    operator SVGPoint() const { return SVGPoint{v1(0), v1(1)}; };

    // ==========================================================================
    // Debug for edge
    // ==========================================================================
    friend std::ostream &operator<<(std::ostream &os, const LinkedEdge &edge) {
      os << "Edge 1:[" << edge.v1(0) << ", " << edge.v1(1) << ", " << edge.v1(2)
         << "], 2:[" << edge.v2(0) << ", " << edge.v2(1) << ", " << edge.v2(2)
         << "]";
      if (edge.mesh != nullptr) {
        os << " ->" << ((edge.owned) ? " OWNING" : "") << " Mesh "
           << edge.mesh->id << " on its " << edgeName(edge.mesh->parent_edge)
           << " edge";
      }

      return os;
    }
  };

  // ==========================================================================
  // Facet description
  // ==========================================================================
  // Facet properties
  ulong id;

  // Linking
  EdgeName parent_edge = NONE;
  math::Vertex n;
  LinkedEdge f12, f23, f31;

  // Flattening
  math::HMat std_mat;
  math::HMat flattening_coef;

  LinkedMesh(microstl::Facet *facet, ulong _id);
  LinkedMesh();

  // ==========================================================================
  // Getters
  // ==========================================================================

  inline LinkedEdge getEdge(EdgeName edge) const {
    switch (edge) {
    case EDGE_23:
      return f23;
    case EDGE_31:
      return f31;
    default:
      return f12;
    }
  }

  /**
   * @brief Get a pointer to the parent
   *
   * @return a pointer to the parent if exist, else nullptr
   */
  inline const LinkedMesh *getParent() const {
    switch (parent_edge) {
    case EDGE_12:
      return f12.mesh;
    case EDGE_23:
      return f23.mesh;
    case EDGE_31:
      return f31.mesh;
    default:
      return nullptr;
    }
  }

  // ==========================================================================
  // Vertex utils
  // ==========================================================================

  /**
   * @brief Return the vertex defining the given edge
   *
   * @param name the edge we want the vertexes of
   */
  VertexPair getEdgeVertex(EdgeName edge) const {
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
  math::Vertex getEdgeDirection(EdgeName edge) const {
    return getEdge(edge).dir();
  };

  /**
   * @brief Get the Edge position in the world.
   *
   * @param edge the edge we will make the vector for
   * @return Vec3 a eigen vector representing the edge position
   */
  math::Vertex getEdgePosition(EdgeName edge) const {
    return getEdge(edge).pos();
  };

  /**
   * @brief Get the Normal vector of the parent of this facet. If there is no
   * parent, return the Z-axis vector.
   *
   * @return an homogenous eigen vector
   */
  math::Vertex getParentNormal() const {
    if (getParent() != nullptr)
      return getParent()->getNormal();
    return math::Vertex{0, 0, 1 / std::sqrt(2), 1 / std::sqrt(2)};
  }

  const math::HMat getParentTrsf() const {
    if (getParent() != nullptr)
      return getParent()->flattening_coef;
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
  math::HMat getHTransform(EdgeName edge) const;

  /**
   * @brief Transform this mesh facet with the given homogenous matrix
   *
   * @param mat the homogenous matrix describing the transform
   */
  void rotate(const math::HMat &mat) {
    f12.v1 = (Vec4)(mat * f12.v1);
    f12.v2 = (Vec4)(mat * f12.v2);
    f23.v1 = (Vec4)(mat * f23.v1);
    f23.v2 = (Vec4)(mat * f23.v2);
    f31.v1 = (Vec4)(mat * f31.v1);
    f31.v2 = (Vec4)(mat * f31.v2);
  }

  /**
   * @brief Compute recursively the transformation to put this face into the
   * world plane and call this function on the owned children.
   */
  void unfoldMesh(ulong depth, ulong max_depth);

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
  std::vector<ulong> linkNeighbours(std::vector<LinkedMesh> &pool);

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
  bool hasSameEdge(LinkedMesh *parent_facet, EdgeName edge);

  // ==========================================================================
  // STL Model Unfold + SVG Export
  // ==========================================================================

  SVGPath getSVGPath() const;

  /**
   * @brief Get the Children Pattern S V G Paths object
   *
   */
  void getChildrenPatternSVGPaths(SVGFigure &figure, int depth = 0,
                                  int max_depth = -1) const;

  // ==========================================================================
  // Debug
  // ==========================================================================
  friend std::ostream &operator<<(std::ostream &os, const LinkedMesh &mesh) {
    os << "Mesh " << mesh.id << " : " << std::endl;
    // if (mesh.f12.owned)
    os << "  - f12 " << mesh.f12 << std::endl;
    // if (mesh.f23.owned)
    os << "  - f23 " << mesh.f23 << std::endl;
    // if (mesh.f31.owned)
    os << "  - f31 " << mesh.f31 << std::endl;
    return os;
  }
};

// ==========================================================================
// Facets Pool
// ==========================================================================

/**
 * @brief Represent a pool of facet that are possibly not linked together
 *
 */
struct LinkedMeshPool : std::vector<LinkedMesh> {
  LinkedMeshPool(unsigned long _size)
      : std::vector<LinkedMesh>(std::vector<LinkedMesh>(_size)) {}
  LinkedMeshPool(microstl::Mesh &mesh);

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
   * @brief Unfold the whole pool, starting from the first element (the root of
   * the linked mesh)
   *
   * @param max_depth the maximum of recursivity (-1 for no limit)
   */
  void unfold(ulong max_depth) { (*this)[root].unfoldMesh(0, max_depth); }

  /**
   * @brief Transform the current linked mesh pool into a figure
   *
   * @return SVGFigure
   */
  SVGFigure makeSVGFigure(int max_depth) {
    SVGFigure figure(0);
    (*this)[root].getChildrenPatternSVGPaths(figure, 0, max_depth);
    return figure;
  }

  /**
   * @brief Print useful informations about the pool. For debug purpose.
   *
   */
  void printInformations();

  friend std::ostream &operator<<(std::ostream &os,
                                  const LinkedMeshPool &pool) {
    os << "Pool : " << std::endl;
    for (const LinkedMesh &mesh : pool) {
      os << mesh;
    }
    return os;
  }

private:
  static constexpr ulong DEFAULT_ROOT{0};
  ulong root = DEFAULT_ROOT;
};

} // namespace kami

#endif
