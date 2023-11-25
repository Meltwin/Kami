#ifndef KAMI_LINKED_POOL
#define KAMI_LINKED_POOL

#include "kami/bin_packing.hpp"
#include "kami/bounds.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"
#include <vector>

namespace kami {

typedef bin::Bin<ILinkedMesh> MeshBin;
typedef bin::Box<ILinkedMesh> MeshBox;
typedef std::vector<MeshBin> MeshBinVector;
typedef std::vector<MeshBox> MeshBoxVector;

// ==========================================================================
// Facets Pool
// ==========================================================================

/**
 * @brief Represent a pool of facet that are possibly not linked together
 *
 */
struct LinkedMeshPool : std::vector<std::shared_ptr<ILinkedMesh>> {
  LinkedMeshPool(unsigned long _size)
      : std::vector<std::shared_ptr<ILinkedMesh>>(
            std::vector<std::shared_ptr<ILinkedMesh>>(_size)) {}
  LinkedMeshPool(microstl::Mesh &mesh);

  // ==========================================================================
  // Linking
  // ==========================================================================

  /**
   * @brief Iterate over the facets to link them together.
   * Iterate over them in their order of linking (i.e. the first one to be
   * linked by a facets are the first one to be iterate on). It works with a
   * "FIFO buffer" filled with the id of the new facets linked and own by the
   * current facet.
   *
   */
  void makeFacetPoolInternalLink();

  // ==========================================================================
  // Unfolding
  // ==========================================================================

  /**
   * @brief Unfold the whole pool, starting from the first element (the root of
   * the linked mesh)
   *
   * @param max_depth the maximum of recursivity (-1 for no limit)
   */
  void unfold(ulong max_depth) { (*this)[root]->unfoldMesh(0, max_depth); }

  /**
   * @brief Rescale the figure into the world by the given factor.
   *
   * @param scaling_factor the scaling factor to apply
   */
  void scaleFigure(double scaling_factor) {
    math::HMat mat;
    mat(0, 0) = scaling_factor;
    mat(1, 1) = scaling_factor;
    mat(2, 2) = scaling_factor;
    (*this)[root]->transform(mat, true, false);
  }

  // ==========================================================================
  // Slicing
  // ==========================================================================

  /**
   * @brief Set the bin format for the slicing / exporting phases.
   *
   * @param _format the format of the bin (default is A4 paper)
   */
  void setBinFormat(bin::BinFormat &_format) { format = _format; }

  /**
   * @brief Slice the children into part to prevent mesh overlapping or parts
   * being too big for the bin to contains.
   */
  MeshBinVector slice();

  // ==========================================================================
  // Exporting
  // ==========================================================================

  /**
   * @brief Organise the boxes (parts of the figures) into several bins of the
   * wanted size. Use the Touch Parameter (TP_RF) algorithm from the following
   * paper:
   *
   * Andrea Lodi, Silvano Martello, Daniele Vigo, (1999) Heuristic and
   * Metaheuristic Approaches for a Class of Two-Dimensional Bin Packing
   * Problems. INFORMS Journal on Computing 11(4):345-357.
   * https://doi.org/10.1287/ijoc.11.4.345
   */
  MeshBinVector binPackingAlgorithm(MeshBoxVector &);

  /**
   * @brief Transform the given bin into a SVG String
   */
  std::string getAsSVGString(MeshBin &, int max_depth,
                             double scale_factor) const;

  // ==========================================================================
  // Debug
  // ==========================================================================

  /**
   * @brief Print useful informations about the pool. For debug purpose.
   */
  void printInformations() const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const LinkedMeshPool &pool) {
    os << "Pool : " << std::endl;
    for (const std::shared_ptr<ILinkedMesh> mesh : pool) {
      os << *mesh;
    }
    return os;
  }

private:
  bin::BinFormat format = bin::PaperA<4>();
  static constexpr ulong DEFAULT_ROOT{0};
  ulong root = DEFAULT_ROOT;
};
} // namespace kami

#endif