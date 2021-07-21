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

// For debugging
inline std::ostream &operator<<(std::ostream &out, const hit_record &rec)
{
  return out << "Hittable:\n"
             << "p: " << rec.p << "\nnormal: " << rec.normal << ", t: " << rec.t << ", u: " << rec.u << ", v: " << rec.v << ", front: " << int(rec.front_face) << std::endl;
}

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

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
  virtual Vec3 random(const Point3 &o) const override;

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

double Translate::pdf_value(const Point3 &o, const Vec3 &v) const
{
  hit_record rec;
  if (!this->hit(Ray(o, v), 0.001, infinity, &rec))
    return 0;
  return ptr->pdf_value(o - offset, v);
}

Vec3 Translate::random(const Point3 &o) const
{
  return ptr->random(o - offset);
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

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
  virtual Vec3 random(const Point3 &o) const override;

public:
  shared_ptr<Hittable> ptr;
  double sin_theta;
  double cos_theta;
  bool hasbox;
  AABB bbox;

private:
  Vec3 rotate(const Vec3 &v) const
  {
    auto rot_v = v;
    rot_v[0] = cos_theta * v[0] + sin_theta * v[2];
    rot_v[2] = -sin_theta * v[0] + cos_theta * v[2];
    return rot_v;
  }

  Vec3 counter_rotate(const Vec3 &v) const
  {
    // "transpose rotation matrix" to reverse rotation
    auto rot_v = v;
    rot_v[0] = cos_theta * v[0] - sin_theta * v[2];
    rot_v[2] = sin_theta * v[0] + cos_theta * v[2];
    return rot_v;
  }
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

      Vec3 v(x, 0, z);
      auto newv = rotate(v);

      min[0] = fmin(min[0], newv.x());
      max[0] = fmax(max[0], newv.x());
      min[2] = fmin(min[2], newv.z());
      max[2] = fmax(max[2], newv.z());
    }
  }

  bbox = AABB(min, max);
}

bool RotateY::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  auto origin = counter_rotate(r.origin());
  auto direction = counter_rotate(r.direction());

  Ray rotated_r(origin, direction, r.time());

  if (!ptr->hit(rotated_r, t_min, t_max, rec))
    return false;

  rec->p = rotate(rec->p);
  rec->set_face_normal(rotated_r, rotate(rec->normal));

  return true;
}

double RotateY::pdf_value(const Point3 &o, const Vec3 &v) const
{
  hit_record rec;
  if (!this->hit(Ray(o, v), 0.001, infinity, &rec))
    return 0;

  auto origin = counter_rotate(o);
  auto direction = counter_rotate(v);

  Ray rotated_r(origin, direction, 0.0);

  return ptr->pdf_value(rotated_r.origin(), rotated_r.direction());
}

Vec3 RotateY::random(const Point3 &o) const
{
  auto origin = counter_rotate(o);
  auto vec = ptr->random(origin);
  auto rotated_vec = rotate(vec);
  return rotated_vec;
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

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override
  {
    return ptr->pdf_value(o, v);
  }

  virtual Vec3 random(const Point3 &o) const override
  {
    return ptr->random(o);
  }

public:
  shared_ptr<Hittable> ptr;
};