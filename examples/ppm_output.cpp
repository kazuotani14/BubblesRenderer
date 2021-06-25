// g++ -std=c++17 -I../src -o ppm_output ppm_output.cpp
//  ./ppm_output > image.ppm && open image.ppm

// pixel values are listed in row-major order

#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"

#include <iostream>

Color ray_color(const Ray &r, const Hittable &world)
{
  hit_record rec;
  if (world.hit(r, 0, infinity, &rec))
  {
    return 0.5 * (rec.normal + Color(1, 1, 1));
  }
  Vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main()
{
  // Image
  const auto aspect_ratio = 16.0 / 9.0;
  const int image_width = 400;
  const int image_height = static_cast<int>(image_width / aspect_ratio);
  const int samples_per_pixel = 100;

  // World
  HittableList world;
  world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));      // "small sphere"
  world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100)); // "ground"

  // Camera
  Camera cam;

  // Render
  std::cout << "P3\n"
            << image_width << ' ' << image_height << "\n255\n";

  for (int row = image_height - 1; row >= 0; --row) // scan from top row down (for ppm format)
  {
    std::cerr << "\rScanlines remaining: " << row << ' ' << std::flush;

    for (int col = 0; col < image_width; ++col)
    {
      Color pixel_color(0, 0, 0);
      for (int s = 0; s < samples_per_pixel; ++s)
      {
        auto u = (col + random_double()) / (image_width - 1);
        auto v = (row + random_double()) / (image_height - 1);
        Ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, world);
      }
      pixel_color /= samples_per_pixel;

      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
}