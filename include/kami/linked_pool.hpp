#ifndef KAMI_LINKED_POOL
#define KAMI_LINKED_POOL

#include "kami/bounds.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"

namespace kami {

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
  void unfold(ulong max_depth) { (*this)[root]->unfoldMesh(0, max_depth); }

  /**
   * @brief Slice the children into part to prevent mesh overlapping
   */
  void slice() { (*this)[root]->sliceChildren(*this); }

  /**
   * @brief Get the bounds for the current linked mesh stored in this pool.
   */
  Bounds getBounds() const;

  /**
   * @brief Transform the current linked mesh pool into a SVG String
   */
  std::string getAsSVGString(int max_depth, double scale_factor) const;

  /**
   * @brief Print useful informations about the pool. For debug purpose.
   *
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
  static constexpr ulong DEFAULT_ROOT{0};
  ulong root = DEFAULT_ROOT;
};
} // namespace kami

#endif