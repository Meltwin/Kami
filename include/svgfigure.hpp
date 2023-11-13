#ifndef SVG_FIGURE
#define SVG_FIGURE

#include <string>
#include <utility>
#include <vector>

namespace kami {

struct Bounds {
  float xmin, xmax;
  float ymin, ymax;

  Bounds &operator+=(const Bounds &);
};

struct SVGPoint {
  float x, y;
};

struct SVGPath : std::vector<SVGPoint> {
public:
  SVGPath() : std::vector<SVGPoint>(0) {}
  SVGPath(unsigned long _size) : std::vector<SVGPoint>(_size) {}
  std::string getAsString(float scale_factor);
  Bounds getBounds();

private:
  static constexpr char TAG_START[] = "<polygon points=\"";
  static constexpr char TAG_END[] =
      "\" stroke=\"black\" fill=\"transparent\" stroke-width=\"1\"/>";
};

struct SVGFigure : std::vector<SVGPath> {
public:
  SVGFigure() : std::vector<SVGPath>(0) {}
  SVGFigure(unsigned long _size) : std::vector<SVGPath>(_size) {}

  std::string getAsString(float scale_factor);
  Bounds getBounds();

private:
  static std::string tag_start(Bounds b, float scale_factor);
  static constexpr char TAG_END[] = "</svg>";
};

} // namespace kami

#endif