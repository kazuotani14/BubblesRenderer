#pragma once

#include "common.h"

#include "aarect.h"
#include "hittable_list.h"

class Box : public Hittable
{
public:
  Box() {}
  Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> ptr);

  virtual bool hit(const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double /*time0*/, double /*time1*/, AABB *output_box) const override
  {
    *output_box = AABB(box_min, box_max);
    return true;
  }

  virtual double pdf_value(const Point3 &origin, const Vec3 &v) const override;
  virtual Vec3 random(const Point3 &origin) const override;

public:
  Point3 box_min;
  Point3 box_max;
  std::vector<shared_ptr<Hittable> > sides;
  HittableList sides_hittable;
};

Box::Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> ptr)
{
  for (int i = 0; i < 3; ++i)
    assert(p0[i] <= p1[i]);

  box_min = p0;
  box_max = p1;

  sides.push_back(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
  sides.push_back(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));

  sides.push_back(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
  sides.push_back(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));

  sides.push_back(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
  sides.push_back(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));

  for (const auto &s : sides)
    sides_hittable.add(s);
}

bool Box::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  return sides_hittable.hit(r, t_min, t_max, rec);
}

double Box::pdf_value(const Point3 &origin, const Vec3 &v) const
{
  double pdf_val = 0;
  for (const auto &s : sides)
    pdf_val += s->pdf_value(origin, v);
  return pdf_val / sides.size();
}

Vec3 Box::random(const Point3 &origin) const
{
  return sides[random_int(0, 5)]->random(origin);
}
