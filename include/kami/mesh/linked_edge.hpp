#ifndef KAMI_LINKED_EDGE
#define KAMI_LINKED_EDGE

#include "kami/export/box_settings.hpp"
#include "kami/export/line_settings.hpp"
#include "kami/export/svg_objects.hpp"
#include "kami/global/arguments.hpp"
#include "kami/math/base_types.hpp"
#include "kami/math/bounds.hpp"
#include "kami/math/edge.hpp"
#include "kami/math/vertex.hpp"
#include <sstream>
#include <vector>

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
  bool hasFixations() const { return has_fixations; }
  bool isInner() const { return (linestyle == LineStyle::INNER); }
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

  void setOtherEdge(ulong n) { other_on = n; }
  ulong getOtherEdge() { return other_on; }

  T *getMesh() const { return mesh; }
  void setMesh(T *p) { mesh = p; }

  // ==========================================================================
  // Overriding
  // ==========================================================================

  void transformEdge(const math::HMat &mat) override {
    v1 = (math::Vec4)(mat * v1);
    v2 = (math::Vec4)(mat * v2);

    // Transforming fixations
    for (ulong i = 0; i < fixations.size(); i++) {
      fixations[i] = (math::Vec4)(mat * fixations[i]);
    }
  }

  math::Bounds getBounds() const override {
    auto b = math::Bounds(std::min(v1(0), v2(0)), std::max(v1(0), v2(0)),
                          std::min(v1(1), v2(1)), std::max(v1(1), v2(1)));

    // Adding fixations
    for (auto &f : fixations) {
      b += math::Bounds(f(0), f(0), f(1), f(1));
    }

    // Adding padding
    b.xmin += ((b.xmin < 0) ? 1 : -1) * out::BOUNDS_PADDING * b.xmin;
    b.xmax += ((b.xmax < 0) ? -1 : 1) * out::BOUNDS_PADDING * b.xmax;
    b.ymin += ((b.ymin < 0) ? 1 : -1) * out::BOUNDS_PADDING * b.ymin;
    b.ymax += ((b.ymax < 0) ? -1 : 1) * out::BOUNDS_PADDING * b.ymax;
    return b;
  }

  // ==========================================================================
  // Fixations
  // ==========================================================================
  void addFixPoint(const math::Vertex &v) {
    has_fixations = true;
    fixations.push_back(v);
  }

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

  void getAsSVGLine(std::stringstream &stream, const args::Args &args) const {
    // Print edge
    svg::line(stream, svg::LineParams{v1(0), v1(1), v2(0), v2(1), linestyle});

    // Print cut number
    if (cut_number != -1) {
      svg::text(
          stream,
          svg::TextParams{(v1(0) + v2(0)) / 2, (v1(1) + v2(1)) / 2, text_size},
          "C" + std::to_string(cut_number));
    }

    // Print fixation
    if (has_fixations && fixations.size() > 0) {
      // V1 -> First point
      svg::line(stream, svg::LineParams{v1(0), v1(1), fixations[0](0),
                                        fixations[0](1), LineStyle::TEST});

      if (args.svg_debug) {
        svg::circle(stream,
                    svg::CircleParams{fixations[0](0), fixations[0](1),
                                      1 * args.resolution, LineStyle::TEST});
      }

      // Add all intermediate points
      for (ulong i = 1; i < fixations.size(); i++) {
        svg::line(stream, svg::LineParams{fixations[i - 1](0),
                                          fixations[i - 1](1), fixations[i](0),
                                          fixations[i](1), LineStyle::TEST}); 

        if (args.svg_debug) {
          svg::circle(stream,
                      svg::CircleParams{fixations[i](0), fixations[i](1),
                                        1 * args.resolution, LineStyle::TEST});
        }
      }
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
  ulong other_on = 0;

  // Fixations
  bool has_fixations = false;
  std::vector<math::Vertex> fixations = std::vector<math::Vertex>(0);

  // Style
  out::LineStyle linestyle = out::LineStyle::PERIMETER;
};
} // namespace kami

#endif