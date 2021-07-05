#pragma once

#include "common.h"

class Texture
{
public:
  virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class SolidColor : public Texture
{
public:
  SolidColor() {}
  SolidColor(const Color &c) : color_value(c) {}

  SolidColor(double red, double green, double blue)
      : SolidColor(Color(red, green, blue)) {}

  virtual Color value(double u, double v, const Vec3 &p) const override
  {
    return color_value;
  }

private:
  Color color_value;
};

class CheckerTexture : public Texture
{
public:
  CheckerTexture() {}

  CheckerTexture(shared_ptr<Texture> _even, shared_ptr<Texture> _odd)
      : even(_even), odd(_odd) {}

  CheckerTexture(const Color &c1, const Color &c2)
      : even(make_shared<SolidColor>(c1)), odd(make_shared<SolidColor>(c2)) {}

  virtual Color value(double u, double v, const Point3 &p) const override
  {
    static constexpr double phase = 10.0;
    auto sines = sin(phase * p.x()) * sin(phase * p.y()) * sin(phase * p.z());
    if (sines < 0)
      return odd->value(u, v, p);
    else
      return even->value(u, v, p);
  }

public:
  shared_ptr<Texture> odd;
  shared_ptr<Texture> even;
};