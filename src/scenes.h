#pragma once

#include "camera.h"
#include "common.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "bvh.h"

#include <optional>
#include <vector>

struct Scene
{
  HittableList objects;
  std::optional<Camera> cam;
  Color background;
};

Camera marble_scene_cam()
{
  Point3 lookfrom(13, 2, 3);
  Point3 lookat(0, 0, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 20.0;
  double aspect_ratio = 16.0 / 9.0;
  double t_start = 0.0;
  double t_end = 1.0;

  Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, t_start, t_end);
  return cam;
}

Scene random_scene()
{
  static constexpr double bounce_t0 = 0.0;
  static constexpr double bounce_t1 = 1.0;

  HittableList world;

  auto ground_material = make_shared<CheckerTexture>(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));
  // auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
  world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(ground_material)));

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

          double bounce_prob = random_double();
          auto center2 = center + ((bounce_prob < 0.2) ? Vec3(0, random_double(0, .5), 0) : Vec3(0, 0, 0));
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

  Scene scene;
  scene.objects = world;
  scene.cam = marble_scene_cam();
  scene.background = Color(0.7, 0.8, 1.0);
  return scene;
}

Scene two_spheres()
{
  HittableList objects;

  auto checker = make_shared<CheckerTexture>(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));

  objects.add(make_shared<Sphere>(Point3(0, -10, 0), 10, make_shared<Lambertian>(checker)));
  objects.add(make_shared<Sphere>(Point3(0, 10, 0), 10, make_shared<Lambertian>(checker)));

  Scene scene;
  scene.objects = objects;
  scene.cam = marble_scene_cam();
  scene.background = Color(0.7, 0.8, 1.0);
  return scene;
}

Scene earth()
{
  auto earth_texture = make_shared<ImageTexture>("images/earthmap.jpeg"); // assumes running in examples dir
  auto earth_surface = make_shared<Lambertian>(earth_texture);
  auto globe = make_shared<Sphere>(Point3(0, 0, 0), 2, earth_surface);

  Scene scene;
  scene.objects = HittableList(globe);
  scene.cam = marble_scene_cam();
  scene.background = Color(0.7, 0.8, 1.0);
  return scene;
}

Scene simple_light()
{
  HittableList objects;

  auto material1 = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, material1));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, material1));

  auto difflight = make_shared<DiffuseLight>(Color(4, 4, 4)); // brighter than 1: more light
  objects.add(make_shared<AARect<0> >(3, 5, 1, 3, -2, difflight));
  objects.add(make_shared<AARect<2> >(3, 5, 1, 3, -2, difflight));
  objects.add(make_shared<Sphere>(Point3(0, 6.5, 0), 1, difflight));

  Scene scene;
  scene.objects = objects;

  Point3 lookfrom(26, 3, 6);
  Point3 lookat(0, 2, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 20.0;
  double aspect_ratio = 16.0 / 9.0;
  double t_start = 0.0;
  double t_end = 1.0;
  scene.cam = Camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, t_start, t_end);

  scene.background = Color(0, 0, 0);
  return scene;
}

Camera cornell_box_cam()
{
  Point3 lookfrom(278, 278, -800);
  Point3 lookat(278, 278, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 40.0;
  double aspect_ratio = 1.0;
  double t_start = 0.0;
  double t_end = 1.0;
  return Camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, t_start, t_end);
}

Scene cornell_box()
{
  HittableList objects;

  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

  objects.add(make_shared<YZRect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<XYRect>(0, 555, 0, 555, 555, white));

  // make sure light is pointing only down
  objects.add(make_shared<FlipFace>(make_shared<XZRect>(213, 343, 227, 332, 554, light)));

  shared_ptr<Hittable> box1 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
  box1 = make_shared<RotateY>(box1, 15);
  box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));
  objects.add(box1);

  shared_ptr<Hittable> box2 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = make_shared<RotateY>(box2, -18);
  box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));
  objects.add(box2);

  Scene scene;
  scene.objects = objects;
  scene.cam = cornell_box_cam();
  scene.background = Color(0, 0, 0);
  return scene;
}

Scene cornell_smoke()
{
  HittableList objects;

  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(7, 7, 7));

  objects.add(make_shared<YZRect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<XZRect>(113, 443, 127, 432, 554, light));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<XYRect>(0, 555, 0, 555, 555, white));

  shared_ptr<Hittable> box1 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 330, 165), white);
  box1 = make_shared<RotateY>(box1, 15);
  box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));

  shared_ptr<Hittable> box2 = make_shared<Box>(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = make_shared<RotateY>(box2, -18);
  box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));

  objects.add(make_shared<ConstantMedium>(box1, 0.01, Color(0, 0, 0)));
  objects.add(make_shared<ConstantMedium>(box2, 0.01, Color(1, 1, 1)));

  Scene scene;
  scene.objects = objects;
  scene.cam = cornell_box_cam();
  scene.background = Color(0, 0, 0);
  return scene;
}

Scene final_scene()
{
  HittableList objects;

  HittableList balls;
  auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));
  const int balls_per_side = 20;
  for (int i = 0; i < balls_per_side; i++)
  {
    for (int j = 0; j < balls_per_side; j++)
    {
      auto w = 100.0;
      auto x0 = -1000.0 + i * w;
      auto z0 = -1000.0 + j * w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_double(1, 101);
      auto z1 = z0 + w;

      balls.add(make_shared<Box>(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
    }
  }

  HittableList ground_boxes;
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++)
  {
    ground_boxes.add(make_shared<Sphere>(Vec3::random(0, 165), 10, white));
  }

  // Add two sets of repeated object sets as independent BVH nodes, since they have a lot of structure
  objects.add(make_shared<BVHNode>(balls, 0, 1));
  objects.add(make_shared<Translate>(
      make_shared<RotateY>(
          make_shared<BVHNode>(ground_boxes, 0.0, 1.0), 15),
      Vec3(-100, 270, 395)));

  // Add rest of objects
  auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
  objects.add(make_shared<XZRect>(123, 423, 147, 412, 554, light));

  auto center1 = Point3(400, 400, 200);
  auto center2 = center1 + Vec3(30, 0, 0);
  auto moving_sphere_material = make_shared<Lambertian>(Color(0.7, 0.3, 0.1));
  objects.add(make_shared<Sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

  objects.add(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
  objects.add(make_shared<Sphere>(
      Point3(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)));

  auto boundary = make_shared<Sphere>(Point3(360, 150, 145), 70, make_shared<Dielectric>(1.5));
  objects.add(boundary);
  objects.add(make_shared<ConstantMedium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));
  boundary = make_shared<Sphere>(Point3(0, 0, 0), 5000, make_shared<Dielectric>(1.5));
  objects.add(make_shared<ConstantMedium>(boundary, .0001, Color(1, 1, 1)));

  auto emat = make_shared<Lambertian>(make_shared<ImageTexture>("images/earthmap.jpeg"));
  objects.add(make_shared<Sphere>(Point3(400, 200, 400), 100, emat));
  // auto pertext = make_shared<noise_texture>(0.1);
  auto red_texture = make_shared<Lambertian>(Color(0.7, 0.1, 0.1));
  objects.add(make_shared<Sphere>(Point3(220, 280, 300), 80, red_texture));

  Scene scene;
  scene.objects = objects;

  Point3 lookfrom(478, 278, -600);
  Point3 lookat(278, 278, 0);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 40.0;
  double aspect_ratio = 1.0;
  double t_start = 0.0;
  double t_end = 1.0;
  scene.cam = Camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, t_start, t_end);

  scene.background = Color(0, 0, 0);
  return scene;
}

/// Transparent cube with one corner at (0, 0, 0) and opposite corner at (size, size, size)
Scene water_in_box(double box_size, double particle_size, const std::vector<Point3> &particle_positions)
{
  HittableList objects;

  const double half_box_size = 0.5 * box_size;
  const double double_box_size = 2 * box_size;

  auto glass = make_shared<Dielectric>(1.0, Color(0.97, 0.97, 0.97));
  shared_ptr<Hittable> box = make_shared<Box>(Point3(0, 0, 0), Point3(box_size, box_size, box_size), glass);
  objects.add(box);

  auto ground = make_shared<Lambertian>(Color(0.7, 0.7, 0.7));
  // auto ground = make_shared<CheckerTexture>(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));
  objects.add(make_shared<XZRect>(-double_box_size, double_box_size, -double_box_size, double_box_size, -0.1, ground));

  // auto water = make_shared<Lambertian>(Color(0.5, 0.5, 1.0));
  auto water = make_shared<Dielectric>(1.3, Color(0.9, 0.9, 1.0));
  HittableList particles;
  for (const auto &p : particle_positions)
  {
    particles.add(make_shared<Sphere>(Point3(p.x(), p.y(), p.z()), particle_size, water));
  }
  objects.add(make_shared<BVHNode>(particles, 0, 1));

  Scene scene;
  scene.objects = objects;

  Point3 lookfrom(-box_size, box_size, -box_size);
  Point3 lookat(-half_box_size, 0.75 * box_size, -half_box_size);
  // lookfrom = Point3(-box_size, half_box_size, half_box_size);
  // lookat = Point3(0, half_box_size, half_box_size);
  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 60.0;
  double aspect_ratio = 1.0;
  double t_start = 0.0;
  double t_end = 1.0;
  scene.cam = Camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, t_start, t_end);

  scene.background = Color(0.7, 0.8, 1.0);
  return scene;
}