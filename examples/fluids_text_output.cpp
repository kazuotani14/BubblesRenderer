// g++ -std=c++17 -O3 -I../src -o fluids_text_output fluids_text_output.cpp && ./fluids_text_output > particle_data.py
// python fluids_viz.p

// Implementation based on: https://lucasschuermann.com/writing/implementing-sph-in-2d
// "Particle-Based Fluid Simulation for Interactive Applications" Muller et al 2003

#include "fluids/particle.h"
#include "timing.h"

#include <limits>
#include <vector>

int main()
{
  const double GRAVITY = 9.8;         // [m/s^2]
  const double REST_DENSITY = 1000.0; // [kg / m^3]
  const double GAS_CONST = 2000.0;
  const double R = 16.0; // [m]
  const double R_SQ = R * R;
  const double R_CU = R * R * R;
  const double MASS = 65.0; // [kg]

  const double POLY6 = 315. / (64. * M_PI * pow(R, 9.));
  const double SPIKY_GRAD = -45. / (M_PI * pow(R, 6.));
  const double VISC = 250.; // viscosity constant
  const double VISC_LAP = 45. / (M_PI * pow(R, 6.));

  const double box_size = 800.0;
  Vec3 box_lb(0, 0, 0);
  Vec3 box_ub(box_size, box_size, box_size);

  const int num_particles = 500;

  const double duration = 1.0;
  const double dt = 0.0008;

  // Initialize particles
  timing::tic();
  std::vector<Particle> particles;
  particles.reserve(num_particles);

  // "dam break" scenario: block of particles in middle
  for (float y = R; y < box_ub[1] - R; y += R)
    for (float x = box_ub[0] / 4; x <= box_ub[0] / 2; x += R)
      if (particles.size() < num_particles)
      {
        Particle p;
        float jitter = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        p.position = Vec3(x + jitter, y, 0);
        p.velocity = Vec3::Zero();
        particles.push_back(p);
      }
  timing::toc("initialization");

  // Simulate
  timing::tic();
  const int num_steps = duration / dt + 1;

  // hacky output for viz; TODO write images at each frame
  std::cout << "num_particles = " << num_particles << std::endl;
  std::cout << "box_size = " << box_size << std::endl;
  std::cout << "import numpy as np\ndata = np.array([" << std::endl;

  for (int i = 0; i < num_steps; ++i)
  {
    const double t = i * dt;

    // Find neighbors
    std::vector<std::vector<int> > neighbor_ids(num_particles);
    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
      for (int n_idx = 0; n_idx < num_particles; ++n_idx)
      {
        if (n_idx != p_idx && (particles[p_idx].position - particles[n_idx].position).length_squared() < R_SQ)
          neighbor_ids[p_idx].push_back(n_idx);
      }

    // Compute density and pressure
    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
    {
      auto &p = particles[p_idx];
      p.density = MASS * POLY6 * pow(R_SQ, 3.); // initialize with density for this particle

      // Incorporate density from neighboring particles
      for (const int n_idx : neighbor_ids[p_idx])
      {
        const double dist_sq = (particles[n_idx].position - p.position).length_squared();

        p.density += MASS * POLY6 * pow(R_SQ - dist_sq, 3.);
      }
      p.density = fmax(p.density, 1e-20); // avoid division by zero later
      p.pressure = GAS_CONST * (p.density - REST_DENSITY);
    }

    // Compute total forces on each particle
    Vec3 F_pressure, F_visc, F_g;
    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
    {
      auto &p = particles[p_idx];

      F_g = Vec3(0, -GRAVITY * p.density * R_CU, 0);

      // compute pressure force
      F_pressure = F_visc = Vec3::Zero();
      for (const int n_idx : neighbor_ids[p_idx])
      {
        const auto &n = particles[n_idx];

        auto vec_np = n.position - p.position;
        auto vec_np_unit = unit_vector(vec_np);
        double dist = vec_np.length();
        assert(dist < R);

        F_pressure += -vec_np_unit * MASS * (p.pressure + n.pressure) / (2. * n.density) * SPIKY_GRAD * pow(R - dist, 2.);
        F_visc += VISC * MASS * (n.velocity - p.velocity) / n.density * VISC_LAP * (R - dist);
      }

      // Add up forces
      p.force = F_g + F_pressure + F_visc;
    }

    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
    {
      auto &p = particles[p_idx];

      // Integrate forces
      // TODO implement better integration scheme
      p.velocity += dt * p.force / p.density;
      p.position += dt * p.velocity;

      // hacky impulse-based collisions for box constraints
      const double restitution_coeff = 0.5;
      if (p.position.y() < R)
      {
        p.velocity[1] = +abs(p.velocity[1]) * restitution_coeff;
        p.position[1] = R;
      }

      if (p.position.y() > box_size - R)
      {
        p.velocity[1] = -abs(p.velocity[1]) * restitution_coeff;
        p.position[1] = box_size - R;
      }

      if (p.position.x() < R)
      {
        p.velocity[0] = +abs(p.velocity[0]) * restitution_coeff;
        p.position[0] = R;
      }

      if (p.position.x() > box_size - R)
      {
        p.velocity[0] = -abs(p.velocity[0]) * restitution_coeff;
        p.position[0] = box_size - R;
      }

      p.position[2] = 0.0;

      std::cout << "[" << p.position << ", " << p.density << "]," << std::endl;
    }
  }
  std::cout << "])" << std::endl;
  timing::toc("simulate");

  return 0;
}