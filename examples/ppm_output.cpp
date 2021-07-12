#include "scenes.h"
#include "bvh.h"
#include "render.h"
#include "camera.h"
#include "timing.h"

#include <iostream>

int main()
{
  // Image
  double aspect_ratio = 16.0 / 9.0;
  int image_width = 300;
  int samples_per_pixel = 300;
  int max_depth = 50;

  // World, camera
  int scene_id = 0;

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
  case 7: // testing box for fluids
    static constexpr double box_size = 800.0;
    static constexpr double half_box_size = 0.5 * box_size;
    static constexpr double particle_size = 16.0;
    world = water_in_box(box_size, particle_size, {{half_box_size, half_box_size, half_box_size}});
    aspect_ratio = 1.0;
    image_width = 200;
    samples_per_pixel = 200;
    background = Color(0.7, 0.8, 1.0);
    vfov = 60.0;
    // lookfrom = Point3(-box_size, half_box_size, half_box_size);
    // lookat = Point3(0, half_box_size, half_box_size);
    lookfrom = Point3(-box_size, box_size, -box_size);
    lookat = Point3(-half_box_size, 0.75 * box_size, -half_box_size);
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
  render(std::cout, world_bvh, cam, image_height, image_width, background, samples_per_pixel, max_depth);
  timing::toc("render");

  return 0;
}