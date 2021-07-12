# TODOs

* for cornell box: how to reduce noise? see "rest of your life" for importance sampling

* speed up bottlenecks
  * rendering: hit
      * parallelize samples or pixels: threads, GPU
      * profile and optimize individual primitives' hit methods
  * fluids: finding neighbors
    * spatial hash
    * At current scale, rendering takes much more time

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
