# SPH Sim

based on: https://sph-tutorial.physics-simulation.org/pdf/SPH_Tutorial.pdf

- using OpenGL graphics and compute shaders

## Overvieww
Smoothed Particle Hydrodynamics involves the use of particles to mimic fluid flow. It uses a lagrangian method meaning each particle holds its own data rather than a grid based method (Eulerian) which holds data at fixed positions on a grid.

To understand how we enforce these particles to produce fluid motion, I will cover these main areas of discussion: 
- Kernels
- Governing Equations
- Neighbourhood Search

## Kernels
As per the name, SPH doesn't handle particles as specific points in the world, rather it effectively sees each particle as a smoothed out sphere in space, with the center of the sphere containing the highest quantity of the particle and where the influence of that particle fades as you move further out from the center. 

A kernel function allows us to take a distance between two particles and work out the 'influence' of one particle on the other. An example of a simple kernel function would be 
