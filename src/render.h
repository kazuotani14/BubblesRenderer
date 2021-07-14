#pragma once

#include "camera.h"
#include "common.h"
#include "color.h"
#include "material.h"
#include "hittable.h"
#include "timing.h"

#include <iostream>
#include <mutex>
#include <thread>

Color ray_color(const Ray &r, const Color &background, const Hittable &world, shared_ptr<Hittable> lights, int depth)
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return Color(0, 0, 0);

  // timing::Timer hit_timer("ray_color/hit");
  hit_record rec;
  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, infinity, &rec))
    return background;
  // hit_timer.stop();

  // timing::Timer emit_timer("ray_color/emit");
  Color attenuation;
  Ray scattered;
  Color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
  // emit_timer.stop();

  // timing::Timer scatter_timer("ray_color/scatter");
  if (!rec.mat_ptr->scatter(r, rec, &attenuation, &scattered))
    return emitted;
  // scatter_timer.stop();

  return emitted + attenuation * ray_color(scattered, background, world, lights, depth - 1);
}

void render(std::ostream &out, const Hittable &world, shared_ptr<Hittable> lights, const Camera &cam, int H, int W, const Color &background, int samples_per_pixel, int max_depth, int num_threads = std::thread::hardware_concurrency(), bool print_progress = true)
{
  std::vector<Color> pixel_values(H * W);
  int num_pixels_done = 0;
  std::mutex color_mutex;

  auto render_rows = [&](int r0, int r1) // inclusive range
  {
    for (int row = r0; row <= r1; ++row)
    {
      for (int col = 0; col < W; ++col)
      {
        Color pixel_color(0, 0, 0);
        for (int s = 0; s < samples_per_pixel; ++s)
        {
          auto u = (col + random_double()) / (W - 1);
          auto v = (row + random_double()) / (H - 1);
          Ray r = cam.get_ray(u, v);
          pixel_color += ray_color(r, background, world, lights, max_depth);
        }
        pixel_color /= samples_per_pixel;

        // pixel values are listed in row-major order in ppm format, from top down
        int c_idx = col + (H - 1 - row) * W;

        std::lock_guard<std::mutex> lock(color_mutex);
        pixel_values[c_idx] = pixel_color;
        if (print_progress)
          std::cerr << "\rPixels done: " << int(double(++num_pixels_done) / (H * W) * 100) << "% " << std::flush;
      }
    }
  };

  if (num_threads == 1)
  {
    render_rows(0, H - 1);
  }
  else
  {
    std::vector<std::thread> threads(num_threads);
    int rows_per_thread = H / num_threads;

    for (int t_idx = 0; t_idx < num_threads; ++t_idx)
    {
      int start_row = t_idx * rows_per_thread;
      int end_row = t_idx == num_threads - 1 ? H - 1 : start_row + rows_per_thread - 1;
      threads[t_idx] = std::thread(render_rows, start_row, end_row);
    }

    for (int t_idx = 0; t_idx < num_threads; ++t_idx)
      threads[t_idx].join();
  }

  out << "P3\n"
      << W << ' ' << H << "\n255\n";
  for (const auto &c : pixel_values)
    write_color(out, c);

  if (print_progress)
    std::cerr << "\nDone.\n";
}