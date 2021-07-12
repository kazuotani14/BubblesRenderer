Current fluids simulation implemention uses Smoothed Particle Hydrodynamics (SPH) algorithm, with implementation based on [this blog post](https://lucasschuermann.com/writing/implementing-sph-in-2d) (original paper: "Particle-Based Fluid Simulation for Interactive Applications" Muller et al 2003). Finding a set of parameters / initial density was difficult, so I mostly copied from the blog post.

Fluids equations: [this video](https://www.youtube.com/watch?v=qsYE1wMEMPA) was helpful.
* Conservation of mass (advection)
* Conservation of momentum (Navier-Stokes equation)

### References

* [Slides on particle-based fluid sim from UCSD](https://cseweb.ucsd.edu/classes/sp19/cse291-d/Files/CSE291_09_ParticleBasedFluids.pdf)
* [Another open-source implementation, from Ryan Guy](http://rlguy.com/sphfluidsim/index.html)
* ["An initiation to SPH", Braune and Lewiner](http://thomas.lewiner.org/pdfs/lucas_wuw.pdf)
* [SPH Survival Kit](http://thomas.lewiner.org/pdfs/lucas_wuw.pdf)
* ["SPH Fluids in Computer Graphics", Ihmsen et al 2014](https://cg.informatik.uni-freiburg.de/publications/2014_EG_SPH_STAR.pdf) -- review paper with more advanced techniques
* [SPlisHSPlasH](https://github.com/InteractiveComputerGraphics/SPlisHSPlasH) -- advanced implementation of SPH and variations