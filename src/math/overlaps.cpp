#include "kami/math/overlaps.hpp"
#include <algorithm>
#include <ostream>

namespace kami::overlaps {

// ==========================================================================
// Overlaps operators
// ==========================================================================

MeshOverlaps MeshOverlaps::operator/(const MeshOverlaps &other) const {
  MeshOverlaps out;
  for (const Overlap &overlap : (*this)) {
    auto elem = std::find_if(
        other.begin(), other.end(),
        [overlap](const Overlap &elem) { return elem == overlap; });
    if (elem != other.end())
      out.push_back(overlap);
  }
  return out;
}

MeshOverlaps MeshOverlaps::operator+(const MeshOverlaps &other) const {
  MeshOverlaps out(*this);
  for (const Overlap &overlap : other) {
    auto elem = std::find_if(
        this->begin(), this->end(),
        [overlap](const Overlap &elem) { return elem == overlap; });
    if (elem == this->end())
      out.push_back(overlap);
  }
  return out;
}

MeshOverlaps MeshOverlaps::operator-(const MeshOverlaps &other) const {
  MeshOverlaps out;
  for (const Overlap &overlap : (*this)) {
    auto elem = std::find_if(
        other.begin(), other.end(),
        [overlap](const Overlap &elem) { return elem == overlap; });
    if (elem == other.end())
      out.push_back(overlap);
  }
  return out;
}

std::ostream &operator<<(std::ostream &os, MeshOverlaps &over) {
  os << "Overlaps : " << std::endl;
  for (auto &elem : over) {
    os << "  - " << elem.id1 << " <-> " << elem.id2 << std::endl;
  }
  return os;
}

} // namespace kami::overlaps