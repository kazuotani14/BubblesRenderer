#pragma once

#include "camera.h"
#include "common.h"
#include "color.h"
#include "material.h"
#include "hittable.h"
#include "pdf.h"
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
  Color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
  // emit_timer.stop();

  // timing::Timer scatter_timer("ray_color/scatter");
  scatter_record srec;
  if (!rec.mat_ptr->scatter(r, rec, &srec))
    return emitted;
  // scatter_timer.stop();

  // Skip importance sampling for specular reflections
  if (srec.is_specular)
    return emitted + srec.attenuation * ray_color(srec.specular_ray, background, world, lights, depth - 1);

  // Importance sampling: scatter pdf of hit material + sample towards lights
  std::vector<shared_ptr<PDF> > pdfs = {srec.pdf_ptr};
  if (lights != nullptr)
    pdfs.push_back(make_shared<HittablePDF>(lights, rec.p));
  MixturePDF mixed_pdf(pdfs);

  auto scattered = Ray(rec.p, mixed_pdf.generate(), r.time());
  const double likelihood_ratio = srec.pdf_ptr->value(scattered.direction()) / mixed_pdf.value(scattered.direction());

  return emitted + srec.attenuation * ray_color(scattered, background, world, lights, depth - 1) * likelihood_ratio;
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
        int num_rejected = 0;
        for (int s = 0; s < samples_per_pixel; ++s)
        {
          auto u = (col + random_double()) / (W - 1);
          auto v = (row + random_double()) / (H - 1);
          Ray r = cam.get_ray(u, v);

          // TODO
          Color sample_color = ray_color(r, background, world, lights, max_depth);

          // Hack to try to get rid of speckles
          static constexpr double max_light = 15;
          const bool nan_color = isnan(sample_color[0]) || isnan(sample_color[1]) || isnan(sample_color[2]);
          const bool high_val_color = sample_color[0] > max_light || sample_color[1] > max_light || sample_color[2] > max_light;
          if (nan_color || high_val_color)
          {
            ++num_rejected;
            continue;
          }

          pixel_color += sample_color;

          // pixel_color += ray_color(r, background, world, lights, max_depth);
        }
        pixel_color /= (samples_per_pixel - num_rejected);

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