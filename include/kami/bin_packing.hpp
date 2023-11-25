#ifndef KAMI_BIN_PACKING
#define KAMI_BIN_PACKING

#include "kami/bounds.hpp"
#include "kami/math.hpp"
#include <algorithm>
#include <ostream>
#include <sstream>
#include <vector>
namespace kami::bin {

#define STHRES math::SIMPLIFICATION_THRESHOLD

struct BinFormat {
  double width, height;

  friend std::ostream &operator<<(std::ostream &os, const BinFormat &format) {
    os << "(" << format.width << ", " << format.height << ")";
    return os;
  }
};

// A Series sizes
constexpr double A0_WIDTH{841};
constexpr double A0_HEIGHT{1189};

template <int N> struct PaperA : public BinFormat {
  PaperA() {
    width = A0_WIDTH;
    height = A0_HEIGHT;
    for (int i = 0; i < N; i++) {
      auto new_height = width / 2;
      width = height;
      height = new_height;
    }
  }
};

struct Edge {
  double x1, y1;
  double x2, y2;

  /**
   * @brief Compute on how much distance the two egdes are overlapping. Consider
   * that the edges are horizontal or vertical only.
   *
   * @param e1 the first edge
   * @param e2 the second edge
   * @return double the distance of overlapping
   */
  static double overlapsLength(const Edge &e1, const Edge &e2) {
    // If the edges are on the same x axis
    if ((e1.x1 == e2.x1) && (e1.x2 == e2.x2)) {
      double start = std::max(std::min(e1.y1, e1.y2), std::min(e2.y1, e2.y2));
      double end = std::min(std::max(e1.y1, e1.y2), std::max(e2.y1, e2.y2));
      return std::fabs(end - start);
    }
    // If the edges are on the same y axis
    else if ((e1.y1 == e2.y1) && (e1.y2 == e2.y2)) {
      double start = std::max(std::min(e1.x1, e1.x2), std::min(e2.x1, e2.x2));
      double end = std::min(std::max(e1.x1, e1.x2), std::max(e2.x1, e2.x2));
      return std::fabs(end - start);
    }
    return 0;
  }
};

template <typename T> struct Box {

  static int getId() {
    static int make_id = 0;
    return make_id++;
  }

  Box(T *mesh_ptr, const Bounds &bounds)
      : root(mesh_ptr), width(bounds.xmax - bounds.xmin),
        height(bounds.ymax - bounds.ymin) {
    id = getId();
  }
  Box(const Box &other)
      : id(other.id), root(other.root), width(other.width),
        height(other.height), x(other.x), y(other.y), rotated(other.rotated) {}

  int id = -1;
  T *root = nullptr;
  double width, height;
  double x = 0, y = 0;
  bool rotated = false;

  double getWidth() const { return (rotated) ? height : width; }
  double getHeight() const { return (rotated) ? width : height; }
  Edge getEdge(int edge) const {
    switch (edge) {
    case 2:
      return Edge{x + getWidth(), y, x + getWidth(), y + getHeight()};
    case 3:
      return Edge{x, y + getHeight(), x + getWidth(), y + getHeight()};
    case 4:
      return Edge{x, y, x, y + getHeight()};
    default:
      return Edge{x, y, x + getWidth(), y};
    }
  }
  bool isColiding(const Box<T> &other) const {
    bool x_separated = ((x + getWidth() <= other.x + STHRES) ||
                        (x >= other.x + other.getWidth() - STHRES));
    bool y_separated = ((y + getHeight() <= other.y + STHRES) ||
                        (y >= other.y + other.getHeight() - STHRES));
    return (!x_separated && !y_separated);
  }

  friend std::ostream &operator<<(std::ostream &os, Box &box) {
    os << " Box (" << box.x << ", " << box.y << ", ";
    os << box.getWidth() << ", " << box.getHeight() << ") ";
    return os;
  }
};

enum CornerType { C1, C2, CX, CY, CXY };

struct Corner {
  CornerType type;
  double x = 0, y = 0;

  Corner() : x(0), y(0), type(C1){};
  Corner(double _x, double _y, CornerType _type) : x(_x), y(_y), type(_type) {}
  Corner(const Corner &other, CornerType _type)
      : x(other.x), y(other.y), type(_type) {}

  static bool compare(Corner c1, Corner c2) {
    return (c1.y <= c2.y) ? (c1.x < c2.x) : false;
  }

  std::string getStrokeColor() const {
    switch (type) {
    case C1:
      return "red";
    case C2:
      return "red";
    case CX:
      return "black";
    case CY:
      return "black";
    case CXY:
      return "black";
    }
    return "black";
  }

  std::string getFillColor() const {
    switch (type) {
    case C1:
      return "blue";
    case C2:
      return "green";
    case CX:
      return "yellow";
    case CY:
      return "pink";
    case CXY:
      return "brown";
    }
    return "transparent";
  }

  friend std::ostream &operator<<(std::ostream &os, const Corner &c) {
    os << "(" << c.x << ", " << c.y << ")";
    return os;
  }
};

template <typename T> struct Bin {

  static int getId() {
    static int make_id = 0;
    return make_id++;
  }

  Bin(const BinFormat &format) : format(format) {
    id = getId();
    corners.resize(0);
    corners.push_back(Corner());
  }

  int id = -1;
  BinFormat format;
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

    // Check for the sides of the bin                6 -> (83.2461, 91.9319)
    // NR(-22.7065) R(-22.7065)

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
          cumulated += Edge::overlapsLength(tempbox.getEdge(temp_edge),
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
      // Make corner 1 (bottom-right) and corner 2 (top-left)
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
      }

      // If they are, add them
      if (c1_on_another && !c1_taken)
        corners.push_back(c1);
      if (c2_in_corner && !c2_taken)
        corners.push_back(c2);
      if (use_cx)
        corners.push_back(cx);
      if (use_cy)
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

} // namespace kami::bin

#endif
