#pragma once

#include "common.h"

struct Particle
{
  Vec3 position;
  Vec3 velocity = Vec3::Zero();
  double density;
  double pressure = 0.0;
  Vec3 force;
};