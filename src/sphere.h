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

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override;

  // TODO add support for motion blur (these should be function of time)
  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
  virtual Vec3 random(const Point3 &o) const override;

  Point3 center(double time) const;

public:
  Point3 center0, center1;
  double time0, time1;
  double radius;
  shared_ptr<Material> mat_ptr;

private:
  static void get_sphere_uv(const Point3 &p, double &u, double &v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1. (y is up)
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = acos(-p.y());
    auto phi = atan2(-p.z(), p.x()) + pi;

    u = phi / (2 * pi);
    v = theta / pi;
  }
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
  get_sphere_uv(outward_normal, rec->u, rec->v);
  rec->mat_ptr = mat_ptr;

  return true;
}

Point3 Sphere::center(double time) const
{
  return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool Sphere::bounding_box(double time0, double time1, AABB *output_box) const
{
  const Point3 center_t0 = center(time0);
  const Point3 center_t1 = center(time1);

  Vec3 min_center(fmin(center_t0.x(), center_t1.x()), fmin(center_t0.y(), center_t1.y()), fmin(center_t0.z(), center_t1.z()));
  Vec3 max_center(fmax(center_t0.x(), center_t1.x()), fmax(center_t0.y(), center_t1.y()), fmax(center_t0.z(), center_t1.z()));

  Vec3 rad3(radius, radius, radius);

  *output_box = AABB(
      min_center - rad3,
      max_center + rad3);
  return true;
}

double Sphere::pdf_value(const Point3 &o, const Vec3 &v) const
{
  hit_record rec;
  if (!this->hit(Ray(o, v), 0.001, infinity, &rec))
    return 0;

  auto cos_theta_max = sqrt(1 - radius * radius / (center0 - o).length_squared());
  auto solid_angle = 2 * pi * (1 - cos_theta_max);

  return 1 / solid_angle;
}

Vec3 Sphere::random(const Point3 &o) const
{
  Vec3 direction = center0 - o;
  auto distance_squared = direction.length_squared();
  OrthonormalBases uvw;
  uvw.build_from_w(direction);
  return uvw.local(random_to_sphere(radius, distance_squared));
}
