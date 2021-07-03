TODOs

* Ray tracing: the next week
  * re-visit 3.4: correctly handle zeros and nans
  * for cornell box: how to reduce noise?

* clean up code -- one pass
  * remove `using`?

* Optimize code
  * use multiple threads. What's the best way to do this?
  * try optimizing: look for hotstops with callgrind?
* Make my own clang-format file
  * references should be attached to type
  * support c++17 (i.e. `std::vector<std::shared_ptr<Hittable> >` shouldn't have a space)