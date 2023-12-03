#ifndef KAMI_LINKED_EDGE
#define KAMI_LINKED_EDGE

#include "kami/export/line_settings.hpp"
#include "kami/export/svg_objects.hpp"
#include "kami/math/edge.hpp"
#include <sstream>

namespace kami {

using namespace out;

// ==========================================================================
// Mesh edge
// ==========================================================================

/**
 * @brief Represent an edge of a polygon
 *
 * @tparam T the mesh type
 */
template <typename T> class LinkedEdge : public math::Edge {
public:
  LinkedEdge(microstl::Vertex _v1, microstl::Vertex _v2) : Edge(_v1, _v2) {}
  LinkedEdge() : Edge() {}

  // ==========================================================================
  // Getters
  // ==========================================================================
  bool isOwned() const { return owned; }
  bool hasCut() const { return cutted; }
  bool nullMesh() const { return mesh == nullptr; }
  void setCutted(bool cut, int cut_n = -1) {
    cutted = cut;
    if (cut) {
      cut_number = (cut_n == -1) ? newCutNumber() : cut_n;
      linestyle = LineStyle::CUTTED;
    }
  }

  int getCutNumber() { return cut_number; }
  void setLineStyle(LineStyle _style) { linestyle = _style; }
  void setTextRatio(double factor) { text_size *= factor; }

  T *getMesh() const { return mesh; }
  void setMesh(T *p) { mesh = p; }

  // ==========================================================================
  // SVG Export
  // ==========================================================================

  void linkEdgeAsOwner(T *p, bool unlinked) {
    if (unlinked) {
      owned = true;
      linestyle = LineStyle::INNER;
    }
    mesh = p;
  }

  void getAsSVGLine(std::stringstream &stream) const {
    svg::line(stream, svg::LineParams{v1(0), v1(1), v2(0), v2(1), linestyle});

    // Print cut number
    if (cut_number != -1) {
      svg::text(
          stream,
          svg::TextParams{(v1(0) + v2(0)) / 2, (v1(1) + v2(1)) / 2, text_size},
          "C" + std::to_string(cut_number));
    }
  }

  // ==========================================================================
  // Debug for edge
  // ==========================================================================
  friend std::ostream &operator<<(std::ostream &os, const LinkedEdge &edge) {
    os << "Edge 1:[" << edge.v1(0) << ", " << edge.v1(1) << ", " << edge.v1(2)
       << "], 2:[" << edge.v2(0) << ", " << edge.v2(1) << ", " << edge.v2(2)
       << "]";
    if (edge.mesh != nullptr) {
      os << " ->" << ((edge.owned) ? " OWNING" : "") << " Mesh "
         << edge.mesh->getUID() << " on its " << edge.mesh->getParentEdgeName()
         << " edge";
    }

    return os;
  }

private:
  static int newCutNumber() {
    static int cut_n = 1;
    return cut_n++;
  }

  // ==========================================================================
  // Edge description
  // ==========================================================================
  // Linking properties
  bool owned = false;
  bool cutted = false;
  int cut_number = -1;
  double text_size = 2;
  T *mesh = nullptr;

  // Style
  out::LineStyle linestyle = out::LineStyle::PERIMETER;
};
} // namespace kami

#endif