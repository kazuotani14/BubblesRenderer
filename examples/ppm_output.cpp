/*
  g++ -std=c++17 -I../src -o ppm_output ppm_output.cpp
  ./ppm_output > image.ppm
  open image.ppm

  g++ -std=c++17 -I../src -o ppm_output ppm_output.cpp && ./ppm_output > image.ppm && open image.ppm
*/

// pixel values are listed in row-major order

#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"

#include <iostream>

static constexpr double bounce_t0 = 0.0;
static constexpr double bounce_t1 = 1.0;

HittableList random_scene()
{
  HittableList world;

  auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
  world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

  static constexpr double ball_r = 0.2;

  for (int a = -11; a < 11; ++a)
  {
    for (int b = -11; b < 11; ++b)
    {
      Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

      if ((center - Point3(4, 0.2, 0)).length() > 0.9)
      {
        shared_ptr<Material> sphere_material;

        auto choose_mat = random_double();
        if (choose_mat < 0.8)
        {
          // diffuse
          auto albedo = Vec3::random() * Vec3::random();
          sphere_material = make_shared<Lambertian>(albedo);
          auto center2 = center + Vec3(0, random_double(0, .5), 0);
          world.add(make_shared<Sphere>(
              center, center2, bounce_t0, bounce_t1, ball_r, sphere_material));
        }
        else if (choose_mat < 0.95)
        {
          // metal
          auto albedo = Vec3::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = make_shared<Metal>(albedo, fuzz);
          world.add(make_shared<Sphere>(center, ball_r, sphere_material));
        }
        else
        {
          // glass
          sphere_material = make_shared<Dielectric>(1.5);
          world.add(make_shared<Sphere>(center, ball_r, sphere_material));
        }
      }
    }
  }

  auto material1 = make_shared<Dielectric>(1.5);
  world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
  world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

  return world;
}

Color ray_color(const Ray &r, const Hittable &world, int depth)
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return Color(0, 0, 0);

  hit_record rec;
  if (world.hit(r, 0.001, infinity, &rec))
  {
    Color attenuation;
    Ray scattered;
    if (rec.mat_ptr->scatter(r, rec, &attenuation, &scattered))
      return attenuation * ray_color(scattered, world, depth - 1);
    else
      return Color(0, 0, 0); // got absorbed
  }

  // "light source"
  Vec3 unit_direction = unit_vector(r.direction());
  double t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
  // return Color(1, 1, 1);
}

int main()
{
  // Image
  const auto aspect_ratio = 16.0 / 9.0;
  const int image_width = 200;
  const int image_height = static_cast<int>(image_width / aspect_ratio);
  const int samples_per_pixel = 100;
  const int max_depth = 50;

  // World
  auto world = random_scene();

  // Camera
  Point3 lookfrom(13, 2, 3);
  Point3 lookat(0, 0, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.1;

  Camera cam(lookfrom, lookat, vup, 20.0, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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
        pixel_color += ray_color(r, world, max_depth);
      }
      pixel_color /= samples_per_pixel;

      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
}