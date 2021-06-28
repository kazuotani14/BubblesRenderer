#pragma once

#include "rtweekend.h"

class Camera
{
public:
  // focal length is implicitly 1.0
  Camera(Point3 lookfrom, Point3 lookat, Vec3 vup, double vfov_deg, double aspect_ratio)
  {
    auto theta = degrees_to_radians(vfov_deg);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    auto w = unit_vector(lookfrom - lookat);
    auto u = unit_vector(cross(vup, w));
    auto v = cross(w, u);

    origin = lookfrom;
    horizontal = viewport_width * u;
    vertical = viewport_height * v;
    lower_left_corner = origin - horizontal / 2 - vertical / 2 - w; // w is where focal length will go
  }

  /// s,t: normalized [0,1] coordinates from lower left in col, row directions respectively
  Ray get_ray(double s, double t) const
  {
    return Ray(origin, lower_left_corner + s * horizontal + t * vertical - origin);
  }

private:
  Point3 origin;
  Point3 lower_left_corner;
  Vec3 horizontal;
  Vec3 vertical;
};