TODOs
* prove derivation of refraction ray in 10.2
* clean up code -- one pass
  * remove `using`?

* Ray tracing: the next week

* Optimize code
  * use multiple threads. What's the best way to do this?
  * try optimizing: look for hotstops with callgrind?
* Make my own clang-format file
  * references should be attached to type
  * support c++17 (i.e. `std::vector<std::shared_ptr<Hittable> >` shouldn't have a space)