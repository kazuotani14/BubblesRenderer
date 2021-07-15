#include "bvh.h"
#include "camera.h"
#include "render.h"
#include "scenes.h"
#include "timing.h"

#include <iostream>

int main()
{
  // Image
  int image_width = 300;
  int samples_per_pixel = 100;
  int max_depth = 50;

  int scene_id = 4;

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
    break;
  case 4: // cornell box
    scene = cornell_box();
    break;
  case 5: // cornell box smoke
    scene = cornell_smoke();
    break;
  case 6:
    scene = cornell_box_hard();
    break;
  case 7:
    scene = final_scene();
    break;
  case 8: // testing box for fluids
    static constexpr double box_size = 800.0;
    static constexpr double half_box_size = 0.5 * box_size;
    static constexpr double particle_size = 16.0;
    scene = water_in_box(box_size, particle_size, {{half_box_size, half_box_size, half_box_size}});
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
  render(std::cout, world_bvh, scene.lights, *scene.cam, image_height, image_width, scene.background, samples_per_pixel, max_depth);
  timing::toc("render");

  timing::print(std::cerr);

  return 0;
}