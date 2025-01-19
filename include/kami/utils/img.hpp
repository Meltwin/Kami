#ifndef IMG_HPP
#define IMG_HPP

#include <SDL_surface.h>

struct KamiImage {
  KamiImage(const uint8_t *_data, const int _width, const int _height)
      : data(_data), width(_width), height(_height) {}

  const uint8_t *data;
  const int width;
  const int height;
};

SDL_Surface *makeImage(const KamiImage &img);

#endif // IMG_HPP
