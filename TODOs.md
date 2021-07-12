# TODOs

* for cornell box: how to reduce noise? see "rest of your life" for importance sampling

* Improve SPH
  * Surface tension: implement color field gradient + marching cubes (requires triangle mesh rendering)

* speed up bottlenecks
  * rendering: hit
      * parallelize samples or pixels: threads, GPU
      * profile and optimize individual primitives' hit methods
  * fluids: at current scale, rendering takes much more time
    * finding neighbors: implement spatial hash table
    * better numerical integration scheme -> use larger timesteps

* Ray tracing: the next week
  * section 9 (volumes) -- derive the ConstantMedium::hit math
  * re-visit 3.4: correctly handle zeros and nans
  * go through section 5: perlin noise

* clean up
  * Add clang-format file
    * references should be attached to type
    * support c++17 (i.e. `std::vector<std::shared_ptr<Hittable> >` shouldn't have a space)
  * add unit tests?

* future projects
  * beach sim: static sand (image texture) to start, then dynamic
