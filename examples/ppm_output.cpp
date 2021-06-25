// g++ -std=c++17 -I../src -o ppm_output ppm_output.cpp
//  ./ppm_output > image.ppm && open image.ppm

// pixel values are listed in row-major order

#include "rtweekend.h"

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

  // World
  HittableList world;
  world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));      // "small sphere"
  world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100)); // "ground"

  // Camera
  auto viewport_height = 2.0;
  auto viewport_width = aspect_ratio * viewport_height; // square pixel assumption
  auto focal_length = 1.0;

  auto origin = Point3(0, 0, 0);
  auto horizontal = Vec3(viewport_width, 0, 0);
  auto vertical = Vec3(0, viewport_height, 0);

  // negative z is into the screen. x is row, y is col
  auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - Vec3(0, 0, focal_length);

  // Render
  std::cout << "P3\n"
            << image_width << ' ' << image_height << "\n255\n";

  for (int row = image_height - 1; row >= 0; --row) // scan from top row down (for ppm format)
  {
    std::cerr << "\rScanlines remaining: " << row << ' ' << std::flush;

    for (int col = 0; col < image_width; ++col)
    {
      auto u = double(col) / (image_width - 1);
      auto v = double(row) / (image_height - 1);
      Ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
      Color pixel_color = ray_color(r, world);
      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
}