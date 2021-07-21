#pragma once

#include "common.h"
#include "hittable.h"
#include "material.h"

#include <array>

class Triangle : public Hittable
{
public:
  using Vertices = std::array<Point3, 3>;

  Triangle() = default;
  Triangle(const Vertices &vertices, shared_ptr<Material> m) : verts(vertices), mat_ptr(m)
  {
    v0v1 = verts[1] - verts[0];
    v0v2 = verts[2] - verts[0];
    area = 0.5 * cross(v0v1, v0v2).length();

    front_normal = cross(v0v1, v0v2); // follow obj convention; vertices defined CCW
  };

  virtual bool hit(
      const Ray &r, double t_min, double t_max, hit_record *rec) const override;

  virtual bool bounding_box(double time0, double time1, AABB *output_box) const override;

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
  virtual Vec3 random(const Point3 &o) const override;

private:
  Vertices verts;
  shared_ptr<Material> mat_ptr;
  Vec3 front_normal;

  // pre-compute for methods
  Vec3 v0v1;
  Vec3 v0v2;
  double area;
};

bool Triangle::hit(const Ray &r, double t_min, double t_max, hit_record *rec) const
{
  // Moller-Trumbore algorithm: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
  const Vec3 pvec = cross(r.direction(), v0v2);
  const double det = dot(v0v1, pvec);

  // ray and triangle are parallel if det is close to 0
  // note: if the determinant is negative the triangle is backfacing
  if (std::abs(det) < 1e-6)
    return false;

  const double inv_det = 1 / det;

  const Vec3 tvec = r.origin() - verts[0];
  const double u = dot(tvec, pvec) * inv_det;
  if (u < 0 || u > 1)
    return false;

  const Vec3 qvec = cross(tvec, v0v1);
  const double v = dot(r.direction(), qvec) * inv_det;
  if (v < 0 || u + v > 1)
    return false;

  const double t = dot(v0v2, qvec) * inv_det;
  if (t < t_min || t > t_max)
    return false;

  rec->t = t;
  rec->p = r.at(rec->t);
  rec->set_face_normal(r, front_normal);
  rec->u = u;
  rec->v = v;
  rec->mat_ptr = mat_ptr;
  return true;
}

bool Triangle::bounding_box(double /*time0*/, double /*time1*/, AABB *output_box) const
{
  Point3 lb(std::min({verts[0].x(), verts[1].x(), verts[2].x()}),
            std::min({verts[0].y(), verts[1].y(), verts[2].y()}),
            std::min({verts[0].z(), verts[1].z(), verts[2].z()}));

  Point3 ub(std::max({verts[0].x(), verts[1].x(), verts[2].x()}),
            std::max({verts[0].y(), verts[1].y(), verts[2].y()}),
            std::max({verts[0].z(), verts[1].z(), verts[2].z()}));

  Vec3 eps(1e-6, 1e-6, 1e-6);
  *output_box = AABB(lb - eps, ub + eps); // add buffer to make sure all boxes have some depth
  return true;
}

double Triangle::pdf_value(const Point3 &o, const Vec3 &v) const
{
  hit_record rec;
  if (!this->hit(Ray(o, v), 0.001, infinity, &rec))
    return 0;

  // Use AARect's solid angle math. Not sure if this is correct
  auto distance_squared = rec.t * rec.t * v.length_squared();
  auto cosine = fabs(dot(v, rec.normal) / v.length());

  const double solid_angle = area * cosine / distance_squared;
  const double pdf_val = 1 / solid_angle;
  return pdf_val;
}

Vec3 Triangle::random(const Point3 &o) const
{
  const Vec3 v0v1 = verts[1] - verts[0];
  const Vec3 v0v2 = verts[2] - verts[0];

  // Sample within parallelogram, reflect into triangle when necessary
  double r1 = random_double();
  double r2 = random_double();

  if (r1 + r2 > 1)
  {
    r1 = 1 - r1;
    r2 = 1 - r2;
  }

  return verts[0] + r1 * v0v1 + r2 * v0v2 - o;
}