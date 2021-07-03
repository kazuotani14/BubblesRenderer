#pragma once

#include "rtweekend.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "aarect.h"

HittableList random_scene()
{
  static constexpr double bounce_t0 = 0.0;
  static constexpr double bounce_t1 = 1.0;

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

HittableList simple_light()
{
  HittableList objects;

  auto material1 = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, material1));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, material1));

  auto difflight = make_shared<DiffuseLight>(Color(4, 4, 4)); // brighter than 1: more light
  objects.add(make_shared<AARect<0> >(3, 5, 1, 3, -2, difflight));
  objects.add(make_shared<AARect<2> >(3, 5, 1, 3, -2, difflight));
  objects.add(make_shared<Sphere>(Point3(0, 6.5, 0), 1, difflight));

  return objects;
}

HittableList cornell_box()
{
  HittableList objects;

  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

  objects.add(make_shared<YZRect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<XZRect>(213, 343, 227, 332, 554, light));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<XYRect>(0, 555, 0, 555, 555, white));

  return objects;
}