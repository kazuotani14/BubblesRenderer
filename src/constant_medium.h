#pragma once

#include "common.h"

#include "hittable.h"
#include "material.h"
#include "texture.h"

class ConstantMedium : public Hittable
{
public:
    // density: chances of a ray scatters at every small distance as it passes through
    ConstantMedium(shared_ptr<Hittable> b, double density, shared_ptr<Texture> a)
        : boundary(b),
          phase_function(make_shared<Isotropic>(a)),
          neg_inv_density(-1 / density)
    {
    }

    ConstantMedium(shared_ptr<Hittable> b, double density, const Color &c)
        : boundary(b),
          phase_function(make_shared<Isotropic>(c)),
          neg_inv_density(-1 / density)
    {
    }

    virtual bool hit(
        const Ray &r, double t_min, double t_max, hit_record *rec) const override;

    virtual bool bounding_box(double time0, double time1, AABB *output_box) const override
    {
        return boundary->bounding_box(time0, time1, output_box);
    }

public:
    shared_ptr<Hittable> boundary;
    shared_ptr<Material> phase_function;
    double neg_inv_density;
};

bool ConstantMedium::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
    hit_record rec1, rec2;

    // do we hit the (convex) boundary?
    if (!boundary->hit(r, -infinity, infinity, &rec1))
        return false;

    // how far is the next boundary hit along the ray?
    if (!boundary->hit(r, rec1.t + 0.0001, infinity, &rec2))
        return false;

    // handle edge cases: hits outside of range, ray starting within volume
    if (rec1.t < t_min)
        rec1.t = t_min;
    if (rec2.t > t_max)
        rec2.t = t_max;

    if (rec1.t >= rec2.t)
        return false;

    if (rec1.t < 0)
        rec1.t = 0;

    // what are our chances of the ray scattering while it's within the volume?
    const auto ray_length = r.direction().length();
    const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
    const auto hit_distance = neg_inv_density * log(random_double()); // log is negative in (0,1)

    // for this sample, ray passed through without hitting
    if (hit_distance > distance_inside_boundary)
        return false;

    // ray is scattered
    rec->t = rec1.t + hit_distance / ray_length;
    rec->p = r.at(rec->t);
    rec->normal = Vec3(1, 0, 0); // arbitrary
    rec->front_face = true;      // also arbitrary
    rec->mat_ptr = phase_function;

    return true;
}