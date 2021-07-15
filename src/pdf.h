#pragma once

#include "common.h"
#include "orthonormal_bases.h"

/// Return random vector in hemisphere wrt z axis
inline Vec3 random_cosine_direction()
{
  auto r1 = random_double();
  auto r2 = random_double();
  auto z = sqrt(1 - r2);

  auto phi = 2 * pi * r1;
  auto x = cos(phi) * sqrt(r2);
  auto y = sin(phi) * sqrt(r2);

  return Vec3(x, y, z);
}

inline Vec3 random_to_sphere(double radius, double distance_squared)
{
  auto r1 = random_double();
  auto r2 = random_double();
  auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

  auto phi = 2 * pi * r1;
  auto x = cos(phi) * sqrt(1 - z * z);
  auto y = sin(phi) * sqrt(1 - z * z);

  return Vec3(x, y, z);
}

/// PDF: probability density function
class PDF
{
public:
  virtual ~PDF() {}

  virtual double value(const Vec3 &direction) const = 0;
  virtual Vec3 generate() const = 0;
};

/// PDF for uniform sampling in a hemisphere, defined by a vector
class CosinePDF : public PDF
{
public:
  CosinePDF(const Vec3 &w) { uvw.build_from_w(w); }

  virtual double value(const Vec3 &direction) const override
  {
    auto cosine = dot(unit_vector(direction), uvw.w());
    return (cosine <= 0) ? 0 : cosine / pi;
  }

  virtual Vec3 generate() const override
  {
    return uvw.local(random_cosine_direction());
  }

public:
  OrthonormalBases uvw;
};

/// PDF for sampling towards a hittable
class HittablePDF : public PDF
{
public:
  HittablePDF(shared_ptr<Hittable> object_ptr, const Point3 &origin) : ptr(object_ptr), o(origin) {}

  virtual double value(const Vec3 &direction) const override
  {
    return ptr->pdf_value(o, direction);
  }

  virtual Vec3 generate() const override
  {
    return ptr->random(o);
  }

public:
  shared_ptr<Hittable> ptr;
  Point3 o;
};

// TODO handle arbitrary number of PDFs and weightings
class MixturePDF : public PDF
{
public:
  MixturePDF(shared_ptr<PDF> p0, shared_ptr<PDF> p1) : p(2)
  {
    p[0] = p0;
    p[1] = p1;
  }

  virtual double value(const Vec3 &direction) const override
  {
    return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
  }

  virtual Vec3 generate() const override
  {
    if (random_double() < 0.5)
      return p[0]->generate();
    else
      return p[1]->generate();
  }

public:
  std::vector<shared_ptr<PDF> > p;
};