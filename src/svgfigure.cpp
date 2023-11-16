#include "kami/svgfigure.hpp"
#include <cmath>
#include <sstream>

namespace kami {

Bounds &Bounds::operator+=(const Bounds &other) {
  if (other.xmin < this->xmin)
    this->xmin = other.xmin;
  if (other.xmax > this->xmax)
    this->xmax = other.xmax;
  if (other.ymin < this->ymin)
    this->ymin = other.ymin;
  if (other.ymax > this->ymax)
    this->ymax = other.ymax;
  return *this;
}

std::string SVGPath::getAsString(double scale_factor) {
  std::stringstream ss;

  ss << TAG_START;
  for (SVGPoint &point : *this) {
    ss << point.x * scale_factor << " " << point.y * scale_factor << " ";
  }
  ss << TAG_END;
  return ss.str();
}

std::string SVGFigure::tag_start(Bounds b, double scale_factor) {
  std::stringstream ss;
  ss << "<svg width=\"" << (b.xmax - b.xmin) * scale_factor << "\"";
  ss << " height=\"" << (b.ymax - b.ymin) * scale_factor << "\"";
  ss << " viewBox=\"" << b.xmin * scale_factor << " " << b.ymin * scale_factor
     << " " << (b.xmax - b.xmin) * scale_factor << " "
     << (b.ymax - b.ymin) * scale_factor << "\"";
  ss << " xmlns=\"http://www.w3.org/2000/svg\">";

  return ss.str();
}

std::string SVGFigure::getAsString(double scale_factor) {
  std::stringstream ss;

  ss << tag_start(getBounds(), scale_factor) << std::endl;
  for (SVGPath &path : *this)
    ss << path.getAsString(scale_factor) << std::endl;
  ss << TAG_END;

  return ss.str();
}

Bounds SVGPath::getBounds() {
  Bounds bounds;
  for (SVGPoint &point : *this) {
    if (point.x < bounds.xmin)
      bounds.xmin = point.x;
    else if (point.x > bounds.xmax)
      bounds.xmax = point.x;

    if (point.y < bounds.ymin)
      bounds.ymin = point.y;
    else if (point.y > bounds.ymax)
      bounds.ymax = point.y;
  }
  return bounds;
}

Bounds SVGFigure::getBounds() {
  Bounds bounds;
  for (SVGPath &path : *this)
    bounds += path.getBounds();
  return bounds;
}

} // namespace kami