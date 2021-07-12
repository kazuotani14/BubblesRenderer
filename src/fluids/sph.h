#pragma once

#include "common.h"

#include <vector>

struct Particle
{
  Vec3 position;
  Vec3 velocity = Vec3::Zero();
  double density;
  double pressure = 0.0;
  Vec3 force;
  double color_field;
};

// SPH fluid sim cparameters/ onstants. Our sim is sensitive to parameters and initial conditions,
// but these parameters have been found to give reasonable results.
static constexpr double REST_DENSITY = 1000.0; // [kg / m^3]
static constexpr double GAS_CONST = 2000.0;
static constexpr double R = 16.0;    // [m]
static constexpr double MASS = 65.0; // [kg]
static constexpr double VISC = 200.; // viscosity constant

static constexpr double R_SQ = R * R;
static constexpr double R_CU = R * R * R;
static constexpr double GRAVITY = 9.8; // [m/s^2]
static const double POLY6 = 315. / (64. * M_PI * pow(R, 9.));
static const double SPIKY_GRAD = -45. / (M_PI * pow(R, 6.));
static const double VISC_LAP = 45. / (M_PI * pow(R, 6.));

// Initialize "block drop" scenario: cube of particles in middle of box
// Gives a reasonable density to initialize SPH sim
inline std::vector<Particle> initBlockDropScenario(const Vec3 &box_lb,
                                                   const Vec3 &box_ub,
                                                   double R,
                                                   int max_num_particles,
                                                   bool constrain_to_xy = false)
{
  for (int i = 0; i < 3; ++i)
    assert(box_ub[i] >= box_lb[i]);

  std::vector<Particle> particles;
  particles.reserve(max_num_particles);

  auto gen_jitter = [=]()
  { return 0.1 * R * static_cast<float>(rand()) / static_cast<float>(RAND_MAX); };

  const Vec3 range = box_ub - box_lb;

  for (float y = 0; y < range[1] - R; y += R)
    for (float x = range[0] / 4; x <= range[0] / 2; x += R)
      for (float z = range[2] / 4; z <= range[2] / 2; z += R)
        if (particles.size() < static_cast<size_t>(max_num_particles))
        {
          Particle p;
          p.position = box_lb + Vec3(x + gen_jitter(), constrain_to_xy ? 0 : y, z + gen_jitter());
          p.velocity = Vec3::Zero();
          particles.push_back(p);
        }

  return particles;
}

/// Impulse-based collisions for box constraints. Modify particle in-place
inline void enforceBoxConstraints(Particle &p, double R, const Vec3 &box_lb, const Vec3 &box_ub)
{
  static constexpr double restitution_coeff = 0.5;

  for (int box_axis = 0; box_axis <= 2; ++box_axis)
  {
    if (p.position[box_axis] < box_lb[box_axis] + R)
    {
      p.velocity[box_axis] = +abs(p.velocity[box_axis]) * restitution_coeff;
      p.position[box_axis] = box_lb[box_axis] + R;
    }

    if (p.position[box_axis] > box_ub[box_axis] - R)
    {
      p.velocity[box_axis] = -abs(p.velocity[box_axis]) * restitution_coeff;
      p.position[box_axis] = box_ub[box_axis] - R;
    }
  }
}