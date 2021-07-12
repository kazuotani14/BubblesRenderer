#pragma once

#include "rtweekend.h"

#include "aarect.h"
#include "hittable_list.h"

class Box : public Hittable
{
public:
  Box() {}
  Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> ptr);

  virtual bool hit(const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override
  {
    *output_box = AABB(box_min, box_max);
    return true;
  }

public:
  Point3 box_min;
  Point3 box_max;
  HittableList sides;
};

Box::Box(const Point3 &p0, const Point3 &p1, shared_ptr<Material> ptr)
{
  box_min = p0;
  box_max = p1;

  sides.add(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
  sides.add(make_shared<XYRect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

  sides.add(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
  sides.add(make_shared<XZRect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

  sides.add(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
  sides.add(make_shared<YZRect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
}

bool Box::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  return sides.hit(r, t_min, t_max, rec);
}