#include "fluids/particle.h"
#include "timing.h"

#include "common.h"
#include "scenes.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "timing.h"

#include <limits>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

Color ray_color(const Ray &r, const Color &background, const Hittable &world, int depth)
{
  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return Color(0, 0, 0);

  hit_record rec;
  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, infinity, &rec))
    return background;

  Color attenuation;
  Ray scattered;
  Color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, &attenuation, &scattered))
    return emitted;

  return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

int main()
{
  // Image
  double aspect_ratio = 1.0;
  int image_width = 200;
  int samples_per_pixel = 200;
  int max_depth = 50;

  int output_mode = 1; // 0=text, 1=image

  Vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperture = 0.0;
  double vfov = 60.0;

  static constexpr double box_size = 600.0;
  static constexpr double half_box_size = 0.5 * box_size;
  static constexpr double particle_size = 16.0;
  Color background = Color(0.7, 0.8, 1.0);
  // lookfrom = Point3(-box_size, half_box_size, half_box_size);
  // lookat = Point3(0, half_box_size, half_box_size);
  Point3 lookfrom = Point3(-box_size, box_size, -box_size);
  Point3 lookat = Point3(-half_box_size, 0.75 * box_size, -half_box_size);

  int image_height = static_cast<int>(image_width / aspect_ratio);

  Camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

  // Fluids sim constants
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

  Vec3 box_lb(0, 0, 0);
  Vec3 box_ub(box_size, box_size, box_size);

  const int num_particles = 3000;

  const double duration = 1.0;
  const double dt = 1e-3; // 0.0008;
  const int num_steps = duration / dt + 1;

  double render_frame_dt = 0.05;
  const int render_step_interval = render_frame_dt / dt;
  const int total_render_frames = num_steps / render_step_interval;
  const int max_render_id_digits = log10(total_render_frames);

  // Initialize particles
  timing::tic();
  std::vector<Particle> particles;
  particles.reserve(num_particles);

  // "dam break" scenario: block of particles in middle
  for (float y = box_ub[0] / 4; y < box_ub[1] - R; y += R)
    for (float x = box_ub[0] / 4; x <= box_ub[0] / 2; x += R)
      for (float z = box_ub[0] / 4; z <= box_ub[0] / 2; z += R)
        if (particles.size() < num_particles)
        {
          Particle p;
          auto gen_jitter = []()
          { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); };
          p.position = Vec3(x + gen_jitter(), y, z + gen_jitter());
          p.velocity = Vec3::Zero();
          particles.push_back(p);
        }
  timing::toc("initialization");

  // Simulate
  timing::tic();

  std::cerr << "num_steps: " << num_steps << std::endl;

  // hacky output for viz; TODO write images at each frame
  if (output_mode == 0)
  {
    std::cout << "num_particles = " << num_particles << std::endl;
    std::cout << "box_size = " << box_size << std::endl;
    std::cout << "import numpy as np\ndata = np.array([" << std::endl;
  }

  // Render
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

      for (int box_axis = 0; box_axis <= 2; ++box_axis)
      {

        if (p.position[box_axis] < R)
        {
          p.velocity[box_axis] = +abs(p.velocity[box_axis]) * restitution_coeff;
          p.velocity[box_axis] = +abs(p.velocity[box_axis]) * restitution_coeff;
        }

        if (p.position[box_axis] > box_size - R)
        {
          p.velocity[box_axis] = -abs(p.velocity[box_axis]) * restitution_coeff;
          p.position[box_axis] = box_size - R;
        }
      }

      // p.position[2] = 0.0;

      if (output_mode == 0)
      {
        std::cout << "[" << p.position << "]," << std::endl;
      }
    }

    if (output_mode == 1 && i % render_step_interval == 0)
    {
      int frame_id = i / render_step_interval;
      int num_frame_digits = std::max(1, int(log10(frame_id))); // handle log10(zero)=-inf
      int num_lead_zeros = max_render_id_digits - num_frame_digits;
      std::cerr << num_frame_digits << std::endl;

      std::string frame_id_str = num_lead_zeros ? std::string(num_lead_zeros, '0') : "" + std::to_string(frame_id);
      std::string file_name = std::string("images/frames/frame_") + frame_id_str + std::string(".ppm");
      std::ofstream outfile_stream(file_name);

      std::cerr << "Rendering frame " << frame_id << " / " << total_render_frames << " at sim step " << i << " to " << file_name << std::endl;

      std::vector<Point3> particle_positions(particles.size());
      for (int pi = 0; pi < particles.size(); ++pi)
        particle_positions[pi] = particles[pi].position;

      HittableList world = fluids_box(box_size, particle_size, particle_positions);
      auto world_bvh = BVHNode(world, /* time0 */ 0, /* time1 */ 9999);

      outfile_stream << "P3\n"
                     << image_width << ' ' << image_height << "\n255\n";

      // pixel values are listed in row-major order
      for (int row = image_height - 1; row >= 0; --row) // scan from top row down (for ppm format)
      {
        std::cerr << "\rScanlines remaining: " << row << ' ' << std::flush;

        for (int col = 0; col < image_width; ++col)
        {
          Color pixel_color(0, 0, 0);
          for (int s = 0; s < samples_per_pixel; ++s)
          {
            auto u = (col + random_double()) / (image_width - 1);
            auto v = (row + random_double()) / (image_height - 1);
            Ray r = cam.get_ray(u, v);
            pixel_color += ray_color(r, background, world_bvh, max_depth);
          }
          pixel_color /= samples_per_pixel;

          write_color(outfile_stream, pixel_color);
        }
      }

      std::cerr << std::endl;
    }
  }

  if (output_mode == 0)
    std::cout << "])" << std::endl;
  timing::toc("simulate");

  std::cerr << "\nDone.\n";

  return 0;
}