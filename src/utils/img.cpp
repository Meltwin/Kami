#include "kami/utils/img.hpp"

SDL_Surface *makeImage(const KamiImage &img) {
  return SDL_CreateRGBSurfaceFrom((void *)img.data, img.width, img.height, 24,
                                  3 * img.width, 0x000000ff, 0x0000ff00,
                                  0x00ff0000, 0);
}
