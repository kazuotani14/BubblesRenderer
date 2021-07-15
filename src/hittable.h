#pragma once

#include "aabb.h"
#include "ray.h"
#include "common.h"

class Material;
struct hit_record
{
  Point3 p;
  Vec3 normal; // normal will always face outwards
  shared_ptr<Material> mat_ptr;
  double t;
  double u;
  double v;

  bool front_face; // whether ray hit from front side (outwards surface) or not
  inline void set_face_normal(const Ray &r, const Vec3 &outward_normal)
  {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class Hittable
{
public:
  virtual ~Hittable() = default;

  // Core methods
  virtual bool hit(const Ray &r, double t_min, double t_max, hit_record *rec) const = 0;
  virtual bool bounding_box(double time0, double time1, AABB *output_box) const = 0;

  // Methods for supporting importance sampling with HittablePDF
  // Compute probability of sampling a given vector from random(o) method
  virtual double pdf_value(const Point3 & /*origin*/, const Vec3 & /*v*/) const
  {
    std::cerr << "Warning: you are calling pdf_value on an unsupported derived class" << std::endl;
    return 0.0;
  }

  // Sample vector from origin to random point within Hittable
  // TODO consider if this should instead just sample a random point within Hittable, and leave the origin subtraction for caller
  virtual Vec3 random(const Vec3 & /*origin*/) const
  {
    std::cerr << "Warning: you are calling random on an unsupported derived class" << std::endl;
    return Vec3(1, 0, 0);
  }
};

class Translate : public Hittable
{
public:
  Translate(shared_ptr<Hittable> p, const Vec3 &displacement)
      : ptr(p), offset(displacement) {}

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override;

public:
  shared_ptr<Hittable> ptr;
  Vec3 offset;
};

bool Translate::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  Ray moved_r(r.origin() - offset, r.direction(), r.time());
  if (!ptr->hit(moved_r, t_min, t_max, rec))
    return false;

  rec->p += offset;
  rec->set_face_normal(moved_r, rec->normal); // do we need this?

  return true;
}

bool Translate::bounding_box(double time0, double time1, AABB *output_box) const
{
  if (!ptr->bounding_box(time0, time1, output_box))
    return false;

  *output_box = AABB(
      output_box->min() + offset,
      output_box->max() + offset);
  return true;
}

/// Rotate about origin. TODO template on axis for x and z
class RotateY : public Hittable
{
public:
  RotateY(shared_ptr<Hittable> p, double angle_degrees);

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double /*time0*/, double /*time1*/, AABB *output_box) const override
  {
    *output_box = bbox;
    return hasbox;
  }

public:
  shared_ptr<Hittable> ptr;
  double sin_theta;
  double cos_theta;
  bool hasbox;
  AABB bbox;
};

RotateY::RotateY(shared_ptr<Hittable> p, double angle_degrees) : ptr(p)
{
  auto radians = degrees_to_radians(angle_degrees);
  sin_theta = sin(radians);
  cos_theta = cos(radians);

  // Get bounding box of rotated object
  hasbox = ptr->bounding_box(0, 1, &bbox);

  Point3 min(infinity, bbox.min().y(), infinity);
  Point3 max(-infinity, bbox.max().y(), -infinity);

  for (int i = 0; i <= 1; i++)
  {
    for (int k = 0; k <= 1; k++)
    {
      // Rotate max and min points. i, k indices are a shorthand way to switch between the two
      auto x = (i == 0) ? bbox.max().x() : bbox.min().x();
      auto z = (k == 0) ? bbox.max().z() : bbox.min().z();

      auto newx = cos_theta * x + sin_theta * z;
      auto newz = -sin_theta * x + cos_theta * z;

      min[0] = fmin(min[0], newx);
      max[0] = fmax(max[0], newx);
      min[2] = fmin(min[2], newz);
      max[2] = fmax(max[2], newz);
    }
  }

  bbox = AABB(min, max);
}

bool RotateY::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  auto origin = r.origin();
  auto direction = r.direction();

  origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
  origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

  direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
  direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

  Ray rotated_r(origin, direction, r.time());

  if (!ptr->hit(rotated_r, t_min, t_max, rec))
    return false;

  auto p = rec->p;
  auto normal = rec->normal;

  // "transpose rotation matrix" to reverse rotation
  p[0] = cos_theta * rec->p[0] + sin_theta * rec->p[2];
  p[2] = -sin_theta * rec->p[0] + cos_theta * rec->p[2];

  normal[0] = cos_theta * rec->normal[0] + sin_theta * rec->normal[2];
  normal[2] = -sin_theta * rec->normal[0] + cos_theta * rec->normal[2];

  rec->p = p;
  rec->set_face_normal(rotated_r, normal);

  return true;
}

class FlipFace : public Hittable
{
public:
  FlipFace(shared_ptr<Hittable> p) : ptr(p) {}

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override
  {
    if (!ptr->hit(r, t_min, t_max, rec))
      return false;

    rec->front_face = !rec->front_face;
    return true;
  }

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override
  {
    return ptr->bounding_box(time0, time1, output_box);
  }

public:
  shared_ptr<Hittable> ptr;
};