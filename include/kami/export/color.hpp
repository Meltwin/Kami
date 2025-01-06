#ifndef KAMI_COLOR
#define KAMI_COLOR

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace kami::color {

typedef unsigned long ulong;

struct Color {
  double r, g, b;

  Color whiten() { return Color{(r + 1) / 2, (g + 1) / 2, (b + 1) / 2}; }

  // Maths
  static double distance2(const Color &c1, const Color &c2) {
    return (c1.r - c2.r) * (c1.r - c2.r) + (c1.g - c2.g) * (c1.g - c2.g) +
           (c1.b - c2.b) * (c1.b - c2.b);
  }
  static double distance(const Color &c1, const Color &c2) {
    return std::sqrt(Color::distance2(c1, c2));
  }

  std::string str() {
    std::stringstream ss;
    ss << "rgb(" << r * 255 << "," << g * 255 << "," << b * 255 << ")";
    return ss.str();
  }
};

class ColorGenerator {
public:
  /**
   * @brief Add a color to the sources.
   *
   * @param c the color
   * @param weight the force of this source
   */
  void addColorSource(const Color &c, const double &weight) {
    colors.push_back(c);
    weights.push_back(weight);
  }

  Color makeNewColor() {
    // Starting random color
    double r = 2, g = 2, b = 2;
    double nr = _rd_dist(_rd_engine);
    double ng = _rd_dist(_rd_engine);
    double nb = _rd_dist(_rd_engine);

    // Gradient descend
    int k = 0;
    double fr, fg, fb;
    double ur, ug, ub, norm;
    while (std::fabs(Color::distance({r, g, b}, {nr, ng, nb})) > GRADIENT_TOL &&
           k < MAX_GRADIENT_ITER) {
      // Update rgb
      r = nr;
      g = ng;
      b = nb;

      // Compute grad force
      fr = 0;
      fg = 0;
      fb = 0;
      for (ulong i = 0; i < colors.size(); i++) {
        // Compute direction
        ur = (r - colors[i].r);
        ug = (g - colors[i].g);
        ub = (b - colors[i].b);
        norm = ur * ur + ug * ug + ub * ub;

        // Compute force
        fr += weights[i] * ur / (norm * norm * norm);
        fg += weights[i] * ug / (norm * norm * norm);
        fb += weights[i] * ub / (norm * norm * norm);
      }

      nr = std::max(0., std::min(nr + fr * GRADIENT_STEP, 1.));
      ng = std::max(0., std::min(ng + fg * GRADIENT_STEP, 1.));
      nb = std::max(0., std::min(nb + fb * GRADIENT_STEP, 1.));

      k += 1;
    }

    colors.push_back({nr, ng, nb});
    weights.push_back(NEW_COLOR_W);

    return Color{nr, ng, nb};
  }

  // --------------------------------------------------------------------------
  // Static methods
  // --------------------------------------------------------------------------

  /**
   * @brief Create a new generator with default sources at corners.
   */
  static ColorGenerator basicGenerator() {
    ColorGenerator gen;
    gen.addColorSource({1, 1, 1}, WHITE_W);
    gen.addColorSource({1, 1, 0}, OTHER_W);
    gen.addColorSource({1, 0, 1}, OTHER_W);
    gen.addColorSource({1, 0, 0}, OTHER_W);
    gen.addColorSource({0, 1, 1}, OTHER_W);
    gen.addColorSource({0, 1, 0}, OTHER_W);
    gen.addColorSource({0, 0, 1}, OTHER_W);
    gen.addColorSource({0, 0, 0}, BLACK_W);
    return gen;
  }

private:
  ColorGenerator() {}

  // Random generator
  std::default_random_engine _rd_engine;
  std::uniform_real_distribution<> _rd_dist{0, 1};

  // Gradient descent
  static constexpr int MAX_GRADIENT_ITER{(int)1E3};
  static constexpr double GRADIENT_TOL{1E-5};
  static constexpr double GRADIENT_STEP{1E-4};

  // Weights
  static constexpr double BLACK_W{3};
  static constexpr double WHITE_W{0.6};
  static constexpr double OTHER_W{0.35};
  static constexpr double NEW_COLOR_W{0.2};

  std::vector<Color> colors;
  std::vector<double> weights;
};

} // namespace kami::color

#endif