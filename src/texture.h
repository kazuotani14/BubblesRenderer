#pragma once

#include "rtweekend.h"

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