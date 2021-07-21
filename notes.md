### Rendering

* Rendering equation
  * cosine term (Lambert's law) is derived via differential areas
    * https://pbr-book.org/3ed-2018/Introduction/Photorealistic_Rendering_and_the_Ray-Tracing_Algorithm#LightDistribution
    * https://pbr-book.org/3ed-2018/Color_and_Radiometry/Radiometry#x1-IrradianceandRadiantExitance
  * https://www.cs.princeton.edu/courses/archive/fall10/cos526/papers/zimmerman98.pdf

* Importance sampling
  * [Karoly Zsolonai's video](https://www.youtube.com/watch?v=zZZ4xW0WaY0&list=PLujxSBD-JXgnGmsn7gEyN28P1DnRZG7qi&index=24) is a good explanation
  * One thing that tripped me up: in contrast to PDFs for discrete random variables, PDFs for continuous random variables can be >1 at any point. As long as the integral over the full domain is 1, it's ok. So if the entire domain (e.g. in steradians solid angle) is <1, then we would expect some parts of the associated PDF to be >1.
    * https://brilliant.org/wiki/continuous-random-variables-probability-density/
    * https://computergraphics.stackexchange.com/questions/9711/confusion-about-light-pdf
  * Current sampling strategy samples uniformly on object surface. [This article](https://schuttejoe.github.io/post/arealightsampling/) argues that sampling uniformly on solid angle gives better results.
  * "Area" as defined in the scatter PDFs isn't surface area in meters^2; it's defined in solid angle (steridians, with polar coordinates).

### Fluids

* SPH
  * One bug I had for Mueller implementation: dividing force by mass instead of density in integration step. Why didn't this work? In this implementation, it seems like we really want to treat particles as "smoothed"

### Misc

* Imagemagick: `convert image.ppm image.jpg`

# TODOs

* doxy core interfaces; it's getting confusing
* make sure all types of hittables/materials are still supported
  * put support for motion blur back into interfaces (Sphere class is the only one for now)
  * add spherical PDF to Isotropic

* Improve SPH
  * Surface tension: implement color field gradient + marching cubes (requires triangle mesh rendering)

* speed up bottlenecks
  * rendering: hit
      * parallelize samples or pixels more: GPU
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
  * soft body sim

### References

* ["Physically Based Rendering" book](https://www.pbrt.org/)
* [Karoly rendering course](https://www.cg.tuwien.ac.at/courses/Rendering/VU.SS2019.html) -- lectures on youtube