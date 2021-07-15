#pragma once

#include "common.h"
#include "orthonormal_bases.h"

#include <vector>

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

class MixturePDF : public PDF
{
public:
  // Assume uniform weighting for now
  MixturePDF(std::vector<shared_ptr<PDF> > pdfs) : p(pdfs), weight(1.0 / pdfs.size())
  {
    assert(!pdfs.empty());
  }

  virtual double value(const Vec3 &direction) const override
  {
    double prob = 0.0;
    for (size_t i = 0; i < p.size(); ++i)
      prob += weight * p[i]->value(direction);
    return prob;
  }

  virtual Vec3 generate() const override
  {
    return p[random_int(0, p.size() - 1)]->generate();
  }

public:
  std::vector<shared_ptr<PDF> > p;
  double weight;
};