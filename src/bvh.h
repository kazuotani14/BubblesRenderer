#pragma once

#include "common.h"

#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

inline bool box_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b, int axis)
{
  AABB box_a;
  AABB box_b;

  if (!a->bounding_box(0, 0, &box_a) || !b->bounding_box(0, 0, &box_b))
    std::cerr << "No bounding box in box_compare" << std::endl;

  return box_a.min().e[axis] < box_b.min().e[axis];
}

bool box_x_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
{
  return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
{
  return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b)
{
  return box_compare(a, b, 2);
}

class BVHNode : public Hittable
{
public:
  BVHNode(const HittableList &list, double time0, double time1)
      : BVHNode(list.objects, 0, list.objects.size(), time0, time1)
  {
  }

  BVHNode(
      const std::vector<shared_ptr<Hittable> > &src_objects,
      size_t start_idx, size_t end_idx, double time0, double time1);

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override;

  shared_ptr<Hittable> left;
  shared_ptr<Hittable> right;
  AABB box;
};

BVHNode::BVHNode(
    const std::vector<shared_ptr<Hittable> > &src_objects,
    size_t start_idx, size_t end_idx, double time0, double time1)
{
  auto objects = src_objects; // Create a modifiable array of the source scene objects

  int axis = random_int(0, 2);
  auto comparator = (axis == 0)   ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

  size_t object_span = end_idx - start_idx;

  if (object_span == 1)
  {
    left = right = objects[start_idx];
  }
  else if (object_span == 2)
  {
    if (comparator(objects[start_idx], objects[start_idx + 1]))
    {
      left = objects[start_idx];
      right = objects[start_idx + 1];
    }
    else
    {
      left = objects[start_idx + 1];
      right = objects[start_idx];
    }
  }
  else
  {
    std::sort(objects.begin() + start_idx, objects.begin() + end_idx, comparator);

    auto mid_idx = start_idx + object_span / 2;
    left = make_shared<BVHNode>(objects, start_idx, mid_idx, time0, time1);
    right = make_shared<BVHNode>(objects, mid_idx, end_idx, time0, time1);
  }

  AABB box_left, box_right;

  if (!left->bounding_box(time0, time1, &box_left) || !right->bounding_box(time0, time1, &box_right))
    std::cerr << "No bounding box in BVHNode constructor." << std::endl;

  box = surrounding_box(box_left, box_right);
}

bool BVHNode::bounding_box(double time0, double time1, AABB *output_box) const
{
  *output_box = box;
  return true;
}

bool BVHNode::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  if (!box.hit(r, t_min, t_max))
    return false;

  bool hit_left = left->hit(r, t_min, t_max, rec);
  bool hit_right = right->hit(r, t_min, hit_left ? rec->t : t_max, rec); // does right hit earlier?

  return hit_left || hit_right;
}
