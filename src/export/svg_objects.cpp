#include "kami/export/svg_objects.hpp"
#include "kami/export/line_settings.hpp"

namespace kami::out {

void svg::polyline(stream &os, const std::vector<double> &x,
                   const std::vector<double> &y, const LineStyle &line,
                   const std::string &fill_color, double opacity) {
  os << "<polygon points=\"";
  ulong min = std::min(x.size(), y.size());
  for (ulong i = 0; i < min; i++)
    os << x[i] << "," << y[i] << ((i == min - 1) ? "" : " ");
  os << "\" ";
  os << "fill=\"" << fill_color << "\" ";
  os << "fill-opacity=\"" << opacity << "\" ";
  appendLineStyle(line, os);
  os << "/>\n";
}

void svg::line(stream &os, const LineParams &p) {
  os << "<line ";
  os << "x1=\"" << p.x1 << "\" ";
  os << "y1=\"" << p.y1 << "\" ";
  os << "x2=\"" << p.x2 << "\" ";
  os << "y2=\"" << p.y2 << "\" ";
  appendLineStyle(p.style, os);
  os << "/>\n";
}

void svg::text(stream &os, const TextParams &p, const std::string &text) {
  os << "<text ";
  os << "x=\"" << p.x << "\" ";
  os << "y=\"" << p.y << "\" ";
  os << "font-size=\"" << p.font_size << "px\">";
  os << text;
  os << "</text>";
}

} // namespace kami::out