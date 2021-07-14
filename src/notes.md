### Rendering

* Rendering equation
  * cosine term (Lambert's law) is derived via differential areas
    * https://pbr-book.org/3ed-2018/Introduction/Photorealistic_Rendering_and_the_Ray-Tracing_Algorithm#LightDistribution
    * https://pbr-book.org/3ed-2018/Color_and_Radiometry/Radiometry#x1-IrradianceandRadiantExitance
  * https://www.cs.princeton.edu/courses/archive/fall10/cos526/papers/zimmerman98.pdf
* Importance sampling
  * [Karoly Zsolonai's video](https://www.youtube.com/watch?v=zZZ4xW0WaY0&list=PLujxSBD-JXgnGmsn7gEyN28P1DnRZG7qi&index=24) is a good explanation


### Fluids

* SPH
  * One bug I had for Mueller implementation: dividing force by mass instead of density in integration step. Why didn't this work?