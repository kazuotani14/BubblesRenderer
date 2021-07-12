# Bubbles Renderer

Simple ray trace renderer based on ["Ray Tracing in One Weekend" tutorial series](https://raytracing.github.io/). Name comes from the fact that this renderer pretty much only supports rendering of spheres (and boxes) at the moment. Repo also contains dynamics simulations + infra to render sim results.

Most of the implementation is in headers right now, to keep things simple because I haven't gotten around to setting up a real build system yet. Instructions:
```
cd examples

# For rendering of hard-coded scenes
g++ -std=c++17 -O3 -I../src -o render2ppm ppm_output.cpp
./render2ppm > image.ppm
open image.ppm # on os x

## ... or in one command
g++ -std=c++17 -O3 -I../src -o render2ppm render_to_ppm.cpp && ./render2ppm > image.ppm && open image.ppm

# For fluids sim rendering
## 2D prototype viz: set output_mode = 0
g++ -std=c++17 -O3 -I../src -o fluids_sim fluids_sim.cpp && ./fluids_sim > particle_data.py
python fluids_viz.py

## 3D gif generation: set output_mode =1
g++ -std=c++17 -O3 -I../src -o fluids_sim fluids_sim.cpp && ./fluids_sim
convert -delay 20 -loop 0 images/frames/*.ppm filename.gif
```

See TODOs.md for future work.

![](./examples/images/final_scene.jpg)

![](./examples/images/blockdrop_3d_2.gif)
