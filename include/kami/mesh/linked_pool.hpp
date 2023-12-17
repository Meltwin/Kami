#ifndef KAMI_LINKED_POOL
#define KAMI_LINKED_POOL

#include "kami/export/paper_format.hpp"
#include "kami/global/arguments.hpp"
#include "kami/global/logging.hpp"
#include "kami/mesh/linked_poly.hpp"
#include "kami/packing/bin.hpp"
#include "kami/packing/box.hpp"
#include <vector>

namespace kami {

typedef packing::Bin<LinkedPolygon> MeshBin;
typedef packing::Box<LinkedPolygon> MeshBox;
typedef std::vector<MeshBin> MeshBinVector;
typedef std::vector<MeshBox> MeshBoxVector;

// ==========================================================================
// Facets Pool
// ==========================================================================

/**
 * @brief Represent a pool of facet that are possibly not linked together
 *
 */
struct LinkedMeshPool : std::vector<std::shared_ptr<LinkedPolygon>> {
  LinkedMeshPool(unsigned long _size)
      : std::vector<std::shared_ptr<LinkedPolygon>>(
            std::vector<std::shared_ptr<LinkedPolygon>>(_size)) {}
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
  void unfold(ulong max_depth) {
    TIMED_UTILS;
    TIMED_SECTION("Unfolding the linked mesh",
                  (*this)[root]->unfoldMesh(0, max_depth));
  }

  /**
   * @brief Rescale the figure into the world by the given factor.
   *
   * @param scaling_factor the scaling factor to apply
   */
  void scaleFigure(double scaling_factor) {
    TIMED_UTILS;
    TIMED_SECTION("Rescaling the mesh", {
      math::HMat mat;
      mat(0, 0) = scaling_factor;
      mat(1, 1) = scaling_factor;
      mat(2, 2) = scaling_factor;
      (*this)[root]->transform(mat, true, false);
    });
  }

  // ==========================================================================
  // Slicing
  // ==========================================================================

  /**
   * @brief Set the bin format for the slicing / exporting phases.
   *
   * @param _format the format of the bin (default is A4 paper)
   */
  void setBinFormat(out::PaperFormat &_format) { format = _format; }

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
  std::string getAsSVGString(MeshBin &, const args::Args &args) const;

  // ==========================================================================
  // Projections
  // ==========================================================================

  /**
   * @brief Project the unfolded mesh onto the given two axes. Print them in the
   * increasing order of the barycenter on the third axis (cross product between
   * the first two).
   *
   * @param ax1 the first axis to project onto
   * @param ax2 the second axis to project onto
   * @return a svg string
   */
  std::string getProjectionAsString(const math::Vec3 &ax1,
                                    const math::Vec3 &ax2,
                                    const args::Args &args);

  /**
   * @brief Project the figure onto the XY plane and a view from the top as an
   * SVG string.
   */
  inline std::string projectOnTop(const args::Args &args) {
    return getProjectionAsString(math::Vec3{1, 0, 0}, math::Vec3{0, 1, 0},
                                 args);
  }

  /**
   * @brief Project the figure onto the XY plane and a view from the bottom as
   * an SVG string.
   */
  inline std::string projectOnBottom(const args::Args &args) {
    return getProjectionAsString(math::Vec3{0, 1, 0}, math::Vec3{1, 0, 0},
                                 args);
  }

  /**
   * @brief Project the figure onto the ZX plane and a view from the right as an
   * SVG string.
   */
  inline std::string projectOnRight(const args::Args &args) {
    return getProjectionAsString(math::Vec3{0, 0, 1}, math::Vec3{1, 0, 0},
                                 args);
  }

  /**
   * @brief Project the figure onto the ZX plane and a view from the left as an
   * SVG string.
   */
  inline std::string projectOnLeft(const args::Args &args) {
    return getProjectionAsString(math::Vec3{1, 0, 0}, math::Vec3{0, 0, 1},
                                 args);
  }

  /**
   * @brief Project the figure onto the YZ plane and a view from the front as an
   * SVG string.
   */
  inline std::string projectOnFront(const args::Args &args) {
    return getProjectionAsString(math::Vec3{0, 1, 0}, math::Vec3{0, 0, 1},
                                 args);
  }

  /**
   * @brief Project the figure onto the YX plane and a view from the back as an
   * SVG string.
   */
  inline std::string projectOnBack(const args::Args &args) {
    return getProjectionAsString(math::Vec3{0, 0, 1}, math::Vec3{0, 1, 0},
                                 args);
  }

  // ==========================================================================
  // Debug
  // ==========================================================================

  /**
   * @brief Print useful informations about the pool. For debug purpose.
   */
  void printInformations() const;

  friend std::ostream &operator<<(std::ostream &os,
                                  const LinkedMeshPool &pool) {
    printStepHeader("Pool faces");
    os << "    Number of faces : " << pool.size() << std::endl;
    for (const std::shared_ptr<LinkedPolygon> mesh : pool) {
      os << *mesh;
    }
    return os;
  }

private:
  out::PaperFormat format = out::PaperA<4>();
  static constexpr ulong DEFAULT_ROOT{0};
  ulong root = DEFAULT_ROOT;

  // Unfold unlinked backup for projection
  std::vector<LinkedPolygon> _unfold_unlinked;
  bool _unfold_transformed = false;
  math::Bounds _unfolded_bounds;
};
} // namespace kami

#endif