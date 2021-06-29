#pragma once

#include "hittable.h"
#include "vec3.h"
#include "material.h"

class Sphere : public Hittable
{
public:
  Sphere() {}
  Sphere(const Point3 &cen, double r, shared_ptr<Material> m)
      : center0(cen), center1(cen), time0(0.0), time1(1.0), radius(r), mat_ptr(m){};
  Sphere(const Point3 &cen0, const Point3 &cen1, double t0, double t1,
         double r, shared_ptr<Material> m)
      : center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m){};

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  Point3 center(double time) const;

public:
  Point3 center0, center1;
  double time0, time1;
  double radius;
  shared_ptr<Material> mat_ptr;
};

bool Sphere::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  auto cur_center = center(r.time());

  Vec3 oc = r.origin() - cur_center;
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if (discriminant < 0)
    return false;
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (-half_b - sqrtd) / a;
  if (root < t_min || root > t_max)
  {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || root > t_max)
      return false;
  }

  rec->t = root;
  rec->p = r.at(rec->t);
  Vec3 outward_normal = (rec->p - cur_center) / radius;
  rec->set_face_normal(r, outward_normal);
  rec->mat_ptr = mat_ptr;

  return true;
}

Point3 Sphere::center(double time) const
{
  return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}