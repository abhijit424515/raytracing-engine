#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"

using color = vec3;

inline double linear_to_gamma(double linear_component) {
  return sqrt(linear_component);
}

color get_final_pixel_color(color pixel_color) {
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Apply the linear to gamma transform.
  r = linear_to_gamma(r);
  g = linear_to_gamma(g);
  b = linear_to_gamma(b);

  // Write the translated [0,255] value of each color component.
  static const interval intensity(0.000, 0.999);
  color c(static_cast<int>(256 * intensity.clamp(r)),
          static_cast<int>(256 * intensity.clamp(g)),
          static_cast<int>(256 * intensity.clamp(b)));
  return c;
}

#endif