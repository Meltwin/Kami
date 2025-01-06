#ifndef KAMI_EXPORT_DISPLAY
#define KAMI_EXPORT_DISPLAY

#include <sstream>

namespace kami::out {

enum class LineStyle { NONE, PERIMETER, INNER, CUTTED };

inline void appendLineStyle(LineStyle line, std::stringstream &ss) {
  switch (line) {
  case LineStyle::NONE:
    ss << "stroke=\"white\" stroke-width=\"0\"";
    break;
  case LineStyle::PERIMETER:
    ss << "stroke=\"black\" stroke-width=\"3\"";
    break;
  case LineStyle::INNER:
    ss << "stroke=\"black\" stroke-width=\"1\"";
    break;
  case LineStyle::CUTTED:
    ss << "stroke=\"purple\" stroke-width=\"4\" stroke-dasharray=\"5,5\"";
    break;
  }
}

} // namespace kami::out

#endif