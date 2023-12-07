#include "kami/export/svg_objects.hpp"

namespace kami::out {

void svg::line(stream &os, const LineParams &p) {
  os << "\t<line ";
  os << "x1=\"" << p.x1 << "\" ";
  os << "y1=\"" << p.y1 << "\" ";
  os << "x2=\"" << p.x2 << "\" ";
  os << "y2=\"" << p.y2 << "\" ";
  appendLineStyle(p.style, os);
  os << "/>\n";
}

void svg::circle(stream &os, const CircleParams &p) {
  os << "\t<circle ";
  os << "cx=\"" << p.x << "\" ";
  os << "cy=\"" << p.y << "\" ";
  os << "r=\"" << p.r << "\" ";
  appendLineStyle(p.style, os);
  os << "/>\n";
}

void svg::text(stream &os, const TextParams &p, const std::string &text) {
  os << "\t<text ";
  os << "x=\"" << p.x << "\" ";
  os << "y=\"" << p.y << "\" ";
  os << "font-size=\"" << p.font_size << "px\">";
  os << text;
  os << "</text>\n";
}

} // namespace kami::out