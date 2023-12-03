#ifndef KAMI_OVERLAPS
#define KAMI_OVERLAPS

#include <ostream>
#include <vector>

namespace kami::overlaps {

struct Overlap {
  ulong id1, id2;

  inline bool operator==(const Overlap &other) const {
    return ((id1 == other.id1 && id2 == other.id2) ||
            (id1 == other.id2 && id2 == other.id1));
  }
};

/**
 * @brief Define a list of overlapping between triangles
 */
struct MeshOverlaps : std::vector<Overlap> {
  MeshOverlaps() : std::vector<Overlap>(0) {}
  MeshOverlaps(const MeshOverlaps &other) : std::vector<Overlap>(other) {}

  /**
   * @brief Return the intersection between the two overlapping list.
   */
  MeshOverlaps operator/(const MeshOverlaps &other) const;

  /**
   * @brief Return the sum of the two overlaps (no double)
   */
  MeshOverlaps operator+(const MeshOverlaps &other) const;

  /**
   * @brief Return the overlaps minus the elements in the other overlaps.
   */
  MeshOverlaps operator-(const MeshOverlaps &other) const;

  friend std::ostream &operator<<(std::ostream &os, MeshOverlaps &overlaps);
};

} // namespace kami::overlaps

#endif