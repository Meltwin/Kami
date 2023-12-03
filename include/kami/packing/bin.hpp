#ifndef KAMI_BIN_PACKING
#define KAMI_BIN_PACKING

#include "kami/export/paper_format.hpp"
#include "kami/math/edge.hpp"
#include "kami/packing/box.hpp"
#include "kami/packing/corner.hpp"
#include <algorithm>
#include <ostream>
#include <sstream>
#include <vector>

namespace kami::packing {

#define STHRES math::SIMPLIFICATION_THRESHOLD

template <typename T> struct Bin {

  static int getId() {
    static int make_id = 0;
    return make_id++;
  }

  Bin(const out::PaperFormat &format) : format(format) {
    id = getId();
    corners.resize(0);
    corners.push_back(Corner());
  }
  Bin() : format(out::PaperA<4>()) {
    id = getId();
    corners.resize(0);
    corners.push_back(Corner());
  }

  int id = -1;
  out::PaperFormat format;
  std::vector<Box<T>> boxes;
  std::vector<Corner> corners;

  double getScore(const ulong corner, const Box<T> &box, bool rotated) {
    Box<T> tempbox = Box<T>(box);
    tempbox.rotated = rotated;
    tempbox.x = corners[corner].x;
    tempbox.y = corners[corner].y;

    double cumulated = 0;
    if ((tempbox.x + tempbox.getWidth() > format.width) ||
        (tempbox.y + tempbox.getHeight() > format.height))
      return -2;

    // Check for the sides of the bin
    if (tempbox.x <= STHRES)
      cumulated += tempbox.getHeight();
    if (tempbox.y <= STHRES)
      cumulated += tempbox.getWidth();
    if (std::fabs(tempbox.x + tempbox.getWidth() - format.width) <= STHRES)
      cumulated += tempbox.getHeight();
    if (std::fabs(tempbox.y + tempbox.getHeight() - format.height) <= STHRES)
      cumulated += tempbox.getWidth();

    // Check for other boxes
    for (auto &other : boxes) {
      // Checking box collisions
      if (tempbox.isColiding(other))
        return -1;

      for (int temp_edge = 0; temp_edge < 4; temp_edge++) {
        for (int other_edge = 0; other_edge < 4; other_edge++) {
          // Check collision
          cumulated += math::Edge::overlapsLength(tempbox.getEdge(temp_edge),
                                                  other.getEdge(other_edge));
        }
      }
    }
    return cumulated / (2 * tempbox.width + 2 * tempbox.height) * 100;
  }

  void putIn(const ulong corner, Box<T> &box, bool rotated) {
    // Change box
    box.x = (corners[corner].x < STHRES) ? 0 : corners[corner].x;
    box.y = (corners[corner].y < STHRES) ? 0 : corners[corner].y;
    box.rotated = rotated;
    boxes.push_back(box);

    // Recompute all corners
    corners.resize(0);
    for (Box<T> &valid_box : boxes) {
      // Make corner 1 (bottom-right) and corner 2 (top-left
      auto c1 = Corner(valid_box.x + valid_box.getWidth(), valid_box.y, C1);
      auto c2 = Corner(valid_box.x, valid_box.y + valid_box.getHeight(), C2);

      // And their projections
      auto cx = Corner(c2, CX);
      auto cy = Corner(c1, CY);

      double saved_x = 0, saved_y = 0;
      for (Box<T> &other : boxes) {
        if (valid_box.id == other.id)
          continue;

        // For Cx
        if ((other.y < cx.y) && (cx.y < other.y + other.getHeight())) {
          saved_x = ((other.x + other.getWidth() > saved_x) &&
                     (other.x + other.getWidth() <= cx.x))
                        ? other.x + other.getWidth()
                        : saved_x;
        }

        // For Cy
        if ((other.x < cy.x) && (cy.x < other.x + other.getWidth())) {
          saved_y = ((other.y + other.getHeight() > saved_y) &&
                     (other.y + other.getHeight() <= cy.y))
                        ? other.y + other.getHeight()
                        : saved_y;
        }
      }
      cx.x = saved_x;
      cy.y = saved_y;

      bool use_cx = !(cx.x == c2.x);
      bool use_cy = !(cy.y == c1.y);

      // Project corners
      bool c1_on_another = false, c1_taken = false;
      bool c2_in_corner = false, c2_taken = false;
      bool cx_taken = false, cy_taken = false;

      // Test if corners are valid
      c1_on_another = (c1.y == 0);
      c2_in_corner = (c2.x == 0);
      for (Box<T> &other : boxes) {
        if (valid_box.id == other.id)
          continue;

        // For C1
        c1_on_another =
            c1_on_another ||
            ((c1.x > other.x) && (c1.x < other.x + other.getWidth()) &&
             (std::fabs(c1.y - other.y - other.getHeight()) < STHRES));
        c1_taken = c1_taken || (std::fabs(c1.x - other.x) < STHRES &&
                                std::fabs(c1.y - other.y) < STHRES);

        // For C2
        c2_in_corner =
            c2_in_corner ||
            ((std::fabs(c2.x - other.x - other.getWidth()) < STHRES) &&
             (c2.y > other.y) && (c2.y < other.y + other.getHeight()));
        c2_taken = c2_taken || (std::fabs(c2.x - other.x) < STHRES &&
                                std::fabs(c2.y - other.y) < STHRES);

        cx_taken = cx_taken || (std::fabs(cx.x - other.x) < STHRES &&
                                std::fabs(cx.y - other.y) < STHRES);
        cy_taken = cy_taken || (std::fabs(cy.x - other.x) < STHRES &&
                                std::fabs(cy.y - other.y) < STHRES);
      }

      // If they are, add them
      if (c1_on_another && !c1_taken)
        corners.push_back(c1);
      if (c2_in_corner && !c2_taken)
        corners.push_back(c2);
      if (use_cx && !cx_taken)
        corners.push_back(cx);
      if (use_cy && !cy_taken)
        corners.push_back(cy);
    }

    std::sort(corners.begin(), corners.end(), Corner::compare);
  }

  std::string printCornerVector() {
    std::stringstream ss;
    ulong index = 0;
    for (Corner &c : corners)
      ss << "\t" << index++ << " -> " << c << std::endl;
    return ss.str();
  }

  friend std::ostream &operator<<(std::ostream &os, const Bin<T> &bin) {
    os << "Bin " << bin.id << " " << bin.format;
    return os;
  }
};

} // namespace kami::packing

#endif
