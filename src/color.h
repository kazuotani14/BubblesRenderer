#pragma once

#include "vec3.h"

#include <iostream>

inline void write_color(std::ostream &out, const Color &pixel_color)
{
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Gamma-correct for gamma=2.0.
  r = sqrt(r);
  g = sqrt(g);
  b = sqrt(b);

  // Write the translated [0,255] value of each color component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}