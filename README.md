# TODOs

* section 9 (volumes)
* re-visit 3.4: correctly handle zeros and nans

* Ray tracing: the next week
  * for cornell box: how to reduce noise? try more samples, more
    * see "rest of your life" for importance sampling
  * go through section 5: perlin noise

* clean up code
  * Optimize code
    * use multiple threads. What's the best way to do this?
    * try optimizing: look for hotstops with callgrind?
  * Make custom clang-format file
    * references should be attached to type
    * support c++17 (i.e. `std::vector<std::shared_ptr<Hittable> >` shouldn't have a space)
  * turn on compiler warnings
  * make CMake file? or waf

* future projects
  * fluid simulation + render
  * beach sim: static sand to start, then dynamic