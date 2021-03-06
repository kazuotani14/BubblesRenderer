#pragma once

#include "common.h"

#include "hittable.h"

// AlignedAxis: axis that plane is defined in. 0, 1, 2 = x, y, z
// variable names in methods use x, y assuming z-aligned, but this class is general to xyz
template <int AlignedAxis = 2>
class AARect : public Hittable
{
public:
  AARect() {}

  AARect(double _x0, double _x1, double _y0, double _y1, double _k,
         shared_ptr<Material> mat)
      : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat)
  {
    if constexpr (AlignedAxis == 2)
    {
      axes = {0, 1};
    }
    else if constexpr (AlignedAxis == 1)
    {
      axes = {0, 2};
    }
    else if constexpr (AlignedAxis == 0)
    {
      axes = {1, 2};
    }
    // TODO handle invalid -- throw exception
  };

  virtual bool hit(const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double /*time0*/, double /*time1*/, AABB *output_box) const override
  {
    // The bounding box must have non-zero width in each dimension, so pad the Z
    // dimension a small amount.
    Point3 lb, ub;
    lb[axes.first] = x0;
    lb[axes.second] = y0;
    lb[AlignedAxis] = k - 0.0001;
    ub[axes.first] = x1;
    ub[axes.second] = y1;
    ub[AlignedAxis] = k + 0.0001;

    *output_box = AABB(lb, ub);
    return true;
  }

  virtual double pdf_value(const Point3 &origin, const Vec3 &v) const override
  {
    hit_record rec;
    if (!this->hit(Ray(origin, v), 0.001, infinity, &rec))
      return 0;

    const double area = (x1 - x0) * (y1 - y0);
    auto distance_squared = rec.t * rec.t * v.length_squared();
    auto cosine = fabs(dot(v, rec.normal) / v.length());

    const double solid_angle = area * cosine / distance_squared;
    const double pdf_val = 1 / solid_angle;
    return pdf_val;
  }

  virtual Vec3 random(const Point3 &origin) const override
  {
    Point3 random_point;
    random_point[axes.first] = random_double(x0, x1);
    random_point[axes.second] = random_double(y0, y1);
    random_point[AlignedAxis] = k;
    return random_point - origin;
  }

public:
  double x0, x1, y0, y1, k; // k is z value
  shared_ptr<Material> mp;

  std::pair<int, int> axes;
};

template <int AlignedAxis>
bool AARect<AlignedAxis>::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  auto t = (k - r.origin()[AlignedAxis]) / r.direction()[AlignedAxis];
  if (t < t_min || t > t_max)
    return false;

  auto x = r.origin()[axes.first] + t * r.direction()[axes.first];
  auto y = r.origin()[axes.second] + t * r.direction()[axes.second];
  if (x < x0 || x > x1 || y < y0 || y > y1)
    return false;

  rec->u = (x - x0) / (x1 - x0);
  rec->v = (y - y0) / (y1 - y0);
  rec->t = t;

  Vec3 pos_normal(0, 0, 0);
  pos_normal[AlignedAxis] = 1; // arbitrary

  rec->set_face_normal(r, pos_normal);
  rec->mat_ptr = mp;
  rec->p = r.at(t);
  return true;
}

using YZRect = AARect<0>;
using XZRect = AARect<1>;
using XYRect = AARect<2>;