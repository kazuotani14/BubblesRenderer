#pragma once

#include "common.h"
#include "orthonormal_bases.h"
#include "pdf.h"
#include "texture.h"

struct hit_record;

struct scatter_record
{
  Ray specular_ray;
  bool is_specular;
  Color attenuation;
  shared_ptr<PDF> pdf_ptr;
};

/// Recall: Color = A * color(direction) * pdf_scatter(direction) / pdf_sampling(direction)
class Material
{
public:
  /**
   * @brief Compute scatter on material
   *
   * @param[in]   r_in  The ray that hit the material
   * @param[in]   rec   More information on hit
   * @param[out]  srec  Scatter result, if any
   *
   * @return Whether hit resulted in a scattered ray or not
   */
  virtual bool scatter(
      const Ray & /*r_in*/, const hit_record & /*rec*/, scatter_record * /*srec*/) const
  {
    return false;
  };

  /**
   * @brief Compute amount of light (color) emitted from hit
   */
  virtual Color emitted(const Ray & /*r_in*/, const hit_record & /*rec*/, double /*u*/, double /*v*/, const Point3 & /*p*/) const
  {
    // most materials aren't lights, so they don't emit any light
    return Color(0, 0, 0);
  }

  virtual ~Material() = default;
};

class Lambertian : public Material
{
public:
  Lambertian(const Color &a) : albedo(make_shared<SolidColor>(a)) {}
  Lambertian(shared_ptr<Texture> a) : albedo(a) {}

  virtual bool scatter(
      const Ray & /*r_in*/, const hit_record &rec, scatter_record *srec) const override
  {
    srec->is_specular = false;
    srec->attenuation = albedo->value(rec.u, rec.v, rec.p);
    srec->pdf_ptr = make_shared<CosinePDF>(rec.normal);
    return true;
  }

public:
  shared_ptr<Texture> albedo;
};

class Metal : public Material
{
public:
  Metal(const Color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(
      const Ray &r_in, const hit_record &rec, scatter_record *srec) const override
  {
    Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    srec->specular_ray = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
    srec->attenuation = albedo;
    srec->is_specular = true; // not strictly true if fuzz is non-zero
    srec->pdf_ptr = nullptr;
    return dot(srec->specular_ray.direction(), rec.normal) > 0;
  }

public:
  Color albedo;
  double fuzz;
};

class Dielectric : public Material
{
public:
  Dielectric(double index_of_refraction, const Color &c = Color(1, 1, 1)) : ir(index_of_refraction), albedo(c) {}

  virtual bool scatter(
      const Ray &r_in, const hit_record &rec, scatter_record *srec) const override
  {
    srec->is_specular = true;
    srec->pdf_ptr = nullptr;
    srec->attenuation = albedo;

    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

    Vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    Vec3 direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
      direction = reflect(unit_direction, rec.normal);
    else
      direction = refract(unit_direction, rec.normal, refraction_ratio);

    srec->specular_ray = Ray(rec.p, direction, r_in.time());
    return true;
  }

public:
  double ir; // Index of Refraction
  Color albedo;

private:
  static double reflectance(double cosine, double ref_idx)
  {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
  }
};

class DiffuseLight : public Material
{
public:
  DiffuseLight(shared_ptr<Texture> a) : emit(a) {}
  DiffuseLight(const Color &c) : emit(make_shared<SolidColor>(c)) {}

  virtual bool scatter(
      const Ray & /*r_in*/, const hit_record & /*rec*/, scatter_record * /*srec*/) const override
  {
    // light hitting this material is absorbed / overwritten
    return false;
  }

  virtual Color emitted(const Ray & /*r_in*/, const hit_record &rec, double u, double v, const Point3 &p) const override
  {
    if (rec.front_face)
      return emit->value(u, v, p);
    else
      return Color(0, 0, 0);
  }

public:
  shared_ptr<Texture> emit;
};

class Isotropic : public Material
{
public:
  Isotropic(const Color &c) : albedo(make_shared<SolidColor>(c)) {}
  Isotropic(shared_ptr<Texture> a) : albedo(a) {}

  virtual bool scatter(
      const Ray &r_in, const hit_record &rec, scatter_record *srec) const override
  {
    srec->is_specular = true; // TODO add spherical uniform PDF here
    srec->pdf_ptr = nullptr;
    srec->attenuation = albedo->value(rec.u, rec.v, rec.p);
    srec->specular_ray = Ray(rec.p, random_in_unit_sphere(), r_in.time());
    return true;
  }

public:
  shared_ptr<Texture> albedo;
};