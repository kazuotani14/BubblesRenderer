#pragma once

#include "camera.h"
#include "common.h"
#include "color.h"
#include "material.h"
#include "hittable.h"

#include <iostream>

Color ray_color(const Ray &r, const Color &background, const Hittable &world, int depth)
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return Color(0, 0, 0);

  hit_record rec;
  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, infinity, &rec))
    return background;

  Color attenuation;
  Ray scattered;
  Color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, &attenuation, &scattered))
    return emitted;

  return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

void render(std::ostream &out, const Hittable &world, const Camera &cam, int H, int W, const Color &background, int samples_per_pixel, int max_depth, bool print_progress = true)
{
  out << "P3\n"
      << W << ' ' << H << "\n255\n";

  // pixel values are listed in row-major order
  for (int row = H - 1; row >= 0; --row) // scan from top row down (for ppm format)
  {
    if (print_progress)
      std::cerr << "\rScanlines remaining: " << row << ' ' << std::flush;

    for (int col = 0; col < W; ++col)
    {
      Color pixel_color(0, 0, 0);
      for (int s = 0; s < samples_per_pixel; ++s)
      {
        auto u = (col + random_double()) / (W - 1);
        auto v = (row + random_double()) / (H - 1);
        Ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world, max_depth);
      }
      pixel_color /= samples_per_pixel;

      write_color(out, pixel_color);
    }
  }

  if (print_progress)
    std::cerr << "\nDone.\n";
}