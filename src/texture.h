#pragma once

#include "common.h"
#include "rtw_stb_image.h"

#include <iostream>

class Texture
{
public:
  virtual Color value(double u, double v, const Point3 &p) const = 0;
  virtual ~Texture() = default;
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
  shared_ptr<Texture> even;
  shared_ptr<Texture> odd;
};

class ImageTexture : public Texture
{
public:
  static const int bytes_per_pixel = 3; // unsigned char (1 byte) x3 for RGB

  ImageTexture()
      : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

  ImageTexture(const char *filename)
  {
    auto components_per_pixel = bytes_per_pixel;

    data = stbi_load(
        filename, &width, &height, &components_per_pixel, components_per_pixel);

    if (!data)
    {
      std::cerr << "ERROR: Could not load texture image file '" << filename << std::endl;
      width = height = 0;
    }

    bytes_per_scanline = bytes_per_pixel * width;
  }

  ~ImageTexture()
  {
    delete data;
  }

  virtual Color value(double u, double v, const Vec3 &p) const override
  {
    // If we have no texture data, then return solid cyan as a debugging aid.
    if (data == nullptr)
      return Color(0, 1, 1);

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = clamp(u, 0.0, 1.0);
    v = 1.0 - clamp(v, 0.0, 1.0); // Flip V to image coordinates

    int i = u * width;
    int j = v * height;

    // Clamp integer mapping, since actual coordinates should be less than 1.0
    if (i >= width)
      i = width - 1;
    if (j >= height)
      j = height - 1;

    static constexpr double color_scale = 1.0 / 255.0;
    unsigned char *pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

    return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
  }

private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
};