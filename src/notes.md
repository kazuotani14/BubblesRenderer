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

### Fluids

* SPH
  * One bug I had for Mueller implementation: dividing force by mass instead of density in integration step. Why didn't this work? In this implementation, it seems like we really want to treat particles as "smoothed"