#pragma once

#include "hittable.h"

#include <memory>
#include <vector>

class HittableList : public Hittable
{
public:
  HittableList() {}
  HittableList(std::shared_ptr<Hittable> object) { add(object); }

  void clear() { objects.clear(); }
  void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

public:
  std::vector<std::shared_ptr<Hittable> > objects;
};

bool HittableList::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  auto t_closest_so_far = t_max;

  for (const auto &object : objects)
  {
    if (object->hit(r, t_min, t_closest_so_far, &temp_rec))
    {
      hit_anything = true;
      t_closest_so_far = temp_rec.t;
      *rec = temp_rec;
    }
  }

  return hit_anything;
}