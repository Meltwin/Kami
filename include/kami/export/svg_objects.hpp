#ifndef KAMI_EXPORT_SVG
#define KAMI_EXPORT_SVG

#include "kami/export/line_settings.hpp"
#include <sstream>

namespace kami::out::svg {

typedef std::stringstream stream;

// ==========================================================================
// Line
// ==========================================================================

/**
 * @brief Structure containing the parameters for drawing a SVG line
 *
 */
struct LineParams {
private:
  double x1, y1;
  double x2, y2;

  LineStyle style;

public:
  LineParams(double _x1, double _y1, double _x2, double _y2, LineStyle _style)
      : x1(_x1), y1(_y1), x2(_x2), y2(_y2), style(_style) {}
  friend void line(stream &, const LineParams &);
};

/**
 * @brief Add a line in the given SVG stream
 */
void line(stream &, const LineParams &);

// ==========================================================================
// Text
// ==========================================================================

struct TextParams {
private:
  double x, y;
  double font_size = 2;

public:
  TextParams(double _x, double _y, double _font = 2)
      : x(_x), y(_y), font_size(_font) {}
  friend void text(stream &, const TextParams &, const std::string &);
};

void text(stream &, const TextParams &, const std::string &text);

} // namespace kami::out::svg

#endif