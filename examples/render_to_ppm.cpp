#include "bvh.h"
#include "camera.h"
#include "render.h"
#include "scenes.h"
#include "timing.h"

#include <iostream>

int main()
{
  // Image
  int image_width = 100;
  int samples_per_pixel = 100;
  int max_depth = 50;

  int scene_id = 0;

  // Build world
  timing::tic();
  Scene scene;
  switch (scene_id)
  {
  case 0: // bouncing marbles
    scene = random_scene();
    break;
  case 1: // checkered spheres
    scene = two_spheres();
    break;
  case 2:
    scene = earth();
    break;
  case 3: // lights in the dark
    scene = simple_light();
    samples_per_pixel = 400;
    break;
  case 4: // cornell box
    scene = cornell_box();
    image_width = 600;
    samples_per_pixel = 200;
    break;
  case 5: // cornell box smoke
    scene = cornell_smoke();
    image_width = 600;
    samples_per_pixel = 200;
    break;
  case 6:
    scene = final_scene();
    image_width = 300;
    samples_per_pixel = 200;
    break;
  case 7: // testing box for fluids
    static constexpr double box_size = 800.0;
    static constexpr double half_box_size = 0.5 * box_size;
    static constexpr double particle_size = 16.0;
    scene = water_in_box(box_size, particle_size, {{half_box_size, half_box_size, half_box_size}});
    image_width = 800;
    samples_per_pixel = 10000;
    break;
  default:
    std::cerr << "Invalid scene id: " << scene_id << std::endl;
    exit(1);
  }
  auto world_bvh = BVHNode(scene.objects, /* time0 */ 0, /* time1 */ 9999);
  timing::toc("build world representation");

  // Render
  int image_height = static_cast<int>(image_width / scene.cam->aspect_ratio);

  timing::tic();
  render(std::cout, world_bvh, *scene.cam, image_height, image_width, scene.background, samples_per_pixel, max_depth);
  timing::toc("render");

  return 0;
}