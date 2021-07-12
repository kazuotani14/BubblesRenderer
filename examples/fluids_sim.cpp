#include "fluids/sph.h"

#include "scenes.h"
#include "bvh.h"
#include "camera.h"
#include "render.h"
#include "timing.h"

#include <limits>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
  const int output_mode = 1; // 0 = text on std::cout (see fluids_viz.py), 1 = images

  // Image params; only matters for output_mode=1. Defaults are coarse
  const int image_width = 100;
  const int samples_per_pixel = 100;
  const int max_depth = 10;

  const double aspect_ratio = 1.0; // this should match the scene's cam. TODO cleaner way
  const int image_height = static_cast<int>(image_width / aspect_ratio);

  const double render_frame_dt = 0.05;

  // Fluid sim
  const double box_size = 700.0;
  const double particle_size = 16.0;
  Vec3 box_lb(0, 0, 0);
  Vec3 box_ub(box_size, box_size, box_size);

  const int num_particles = 3000;

  const double duration = 1.0;
  const double dt = 1e-3; // 0.0008;
  const int num_steps = duration / dt + 1;
  const bool constrain_to_xy = false;
  std::cerr << "num_steps: " << num_steps << std::endl;

  const int render_step_interval = render_frame_dt / dt;
  const int total_render_frames = num_steps / render_step_interval;
  const int max_render_id_digits = num_digits(total_render_frames);

  // Initialize particles
  timing::Timer init_timer("initialization");
  std::vector<Particle> particles = initBlockDropScenario(box_lb, box_ub, R, num_particles, constrain_to_xy);
  init_timer.stop();

  // Simulate
  timing::Timer sim_timer("full_sim");

  // hacky output for viz/debugging
  if (output_mode == 0)
  {
    std::cout << "num_particles = " << num_particles << std::endl;
    std::cout << "box_size = " << box_size << std::endl;
    std::cout << "import numpy as np\ndata = np.array([" << std::endl;
  }

  // Render
  for (int i = 0; i < num_steps; ++i)
  {
    // Find neighbors
    std::vector<std::vector<int> > neighbor_ids(num_particles);
    timing::Timer n_timer("find_neighbors");
    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
      for (int n_idx = 0; n_idx < num_particles; ++n_idx)
      {
        if (n_idx != p_idx && (particles[p_idx].position - particles[n_idx].position).length_squared() < R_SQ)
          neighbor_ids[p_idx].push_back(n_idx);
      }
    n_timer.stop();

    // Compute density and pressure
    timing::Timer dp_timer("density_pressure");
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
    dp_timer.stop();

    // Compute total forces on each particle
    timing::Timer f_timer("forces");
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
    f_timer.stop();

    // Integrate forces into motion
    timing::Timer i_timer("integration");
    for (int p_idx = 0; p_idx < num_particles; ++p_idx)
    {
      auto &p = particles[p_idx];

      // Integrate forces
      // TODO implement better integration scheme
      p.velocity += dt * p.force / p.density;
      p.position += dt * p.velocity;

      enforceBoxConstraints(p, R, box_lb, box_ub);

      if (constrain_to_xy)
      {
        p.position[2] = 0.0;
        p.velocity[2] = 0.0;
      }
    }
    i_timer.stop();

    // Output results
    timing::Timer o_timer("output");
    if (output_mode == 0)
    {
      for (const auto &p : particles)
        std::cout << "[" << p.position << "]," << std::endl;
    }
    else if (output_mode == 1 && i % render_step_interval == 0)
    {
      const int frame_id = i / render_step_interval;
      const int num_lead_zeros = max_render_id_digits - num_digits(frame_id);
      const std::string frame_id_str = std::string(num_lead_zeros, '0') + std::to_string(frame_id);
      const std::string file_name = std::string("examples/images/frame_") + frame_id_str + std::string(".ppm");
      std::ofstream outfile_stream(file_name);

      std::cout << "Rendering frame " << frame_id << " / " << total_render_frames << " at sim step " << i << " to " << file_name << std::endl;

      std::vector<Point3> particle_positions(particles.size());
      for (int pi = 0; pi < static_cast<int>(particles.size()); ++pi)
        particle_positions[pi] = particles[pi].position;

      Scene scene = water_in_box(box_size, particle_size, particle_positions);
      auto world_bvh = BVHNode(scene.objects, /* time0 */ 0, /* time1 */ 9999);

      render(outfile_stream, world_bvh, *scene.cam, image_height, image_width, scene.background, samples_per_pixel, max_depth);
    }
    o_timer.stop();
  }

  if (output_mode == 0)
    std::cout << "])" << std::endl;

  sim_timer.stop();
  timing::print(std::cerr);

  return 0;
}