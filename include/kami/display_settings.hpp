#ifndef KAMI_DISPLAY
#define KAMI_DISPLAY

#include <sstream>
namespace kami {

enum class SVGLineWidth { NONE, PERIMETER, INNER, CUTTED };

inline void appendLineStyle(SVGLineWidth line, std::stringstream &ss) {
  switch (line) {
  case SVGLineWidth::NONE:
    ss << "";
    break;
  case SVGLineWidth::PERIMETER:
    ss << "stroke=\"black\" stroke-width=\"3\"";
    break;
  case SVGLineWidth::INNER:
    ss << "stroke=\"black\" stroke-width=\"1\"";
    break;
  case SVGLineWidth::CUTTED:
    ss << "stroke=\"purple\" stroke-width=\"4\" stroke-dasharray=\"5,5\"";
    break;
  }
}

} // namespace kami

#endif