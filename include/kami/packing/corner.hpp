#ifndef KAMI_PACKING_CORNER
#define KAMI_PACKING_CORNER

#include <ostream>

namespace kami::packing {

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
} // namespace kami::packing

#endif