# TODOs

* Ray tracing: the next week
  * go through section 5: perlin noise
  * section 9 (volumes) -- understand the ConstantMedium::hit math
  * re-visit 3.4: correctly handle zeros and nans

* for cornell box: how to reduce noise? see "rest of your life" for importance sampling

* clean up code
  * Optimize code
    * use multiple threads. What's the best way to do this?
    * try optimizing: look for hotstops with callgrind?
  * Make custom clang-format file
    * references should be attached to type
    * support c++17 (i.e. `std::vector<std::shared_ptr<Hittable> >` shouldn't have a space)
  * turn on compiler warnings
  * make CMake file? or try waf

* future projects
  * fluid simulation + render
  * beach sim: static sand (image texture) to start, then dynamic
