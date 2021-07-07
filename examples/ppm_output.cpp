#include "common.h"
#include "scenes.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "timing.h"

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

int main()
{
  // Image
  double aspect_ratio = 16.0 / 9.0;
  int image_width = 480;
  int samples_per_pixel = 300;
  int max_depth = 50;

  // World, camera
  int scene_id = 6;

  // defaults (for sphere scenes)
  Color background(0.7, 0.8, 1.0);
  Point3 lookfrom(13, 2, 3);
  Point3 lookat(0, 0, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 20.0;

  timing::tic();

  HittableList world;
  switch (scene_id)
  {
  case 0: // bouncing marbles
    world = random_scene();
    break;
  case 1: // checkered spheres
    world = two_spheres();
    break;
  case 2:
    world = earth();
    break;
  case 3: // lights in the dark
    world = simple_light();
    background = Color(0, 0, 0);
    samples_per_pixel = 400;
    lookfrom = Point3(26, 3, 6);
    lookat = Point3(0, 2, 0);
    break;
  case 4: // cornell box
    world = cornell_box();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    background = Color(0, 0, 0);
    lookfrom = Point3(278, 278, -800);
    lookat = Point3(278, 278, 0);
    vfov = 40.0;
    break;
  case 5: // cornell box smoke
    world = cornell_smoke();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    background = Color(0, 0, 0);
    lookfrom = Point3(278, 278, -800);
    lookat = Point3(278, 278, 0);
    vfov = 40.0;
    break;
  case 6:
    world = final_scene();
    aspect_ratio = 1.0;
    image_width = 800;
    samples_per_pixel = 10000;
    background = Color(0, 0, 0);
    lookfrom = Point3(478, 278, -600);
    lookat = Point3(278, 278, 0);
    vfov = 40.0;
    break;
  default:
    std::cerr << "Invalid scene id: " << scene_id << std::endl;
    exit(1);
  }

  auto world_bvh = BVHNode(world, /* time0 */ 0, /* time1 */ 9999);
  timing::toc("build world representation");

  int image_height = static_cast<int>(image_width / aspect_ratio);

  Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

  // Render
  timing::tic();
  std::cout << "P3\n"
            << image_width << ' ' << image_height << "\n255\n";

  // pixel values are listed in row-major order
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
        pixel_color += ray_color(r, background, world_bvh, max_depth);
      }
      pixel_color /= samples_per_pixel;

      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone.\n";
  timing::toc("render");
}