# SPH Sim

based on: https://sph-tutorial.physics-simulation.org/pdf/SPH_Tutorial.pdf

- using OpenGL graphics and compute shaders

# Build

# Theory
Smoothed Particle Hydrodynamics involves the use of particles to mimic fluid flow. It is a lagrangian method meaning each particle holds its own data rather than having a grid based method (Eulerian) which holds data at fixed positions.

## Kernels
As per the name, SPH doesn't handle particles as specific points in the world, rather it effectively sees each particle as a smoothed out sphere in space, with the center of the sphere containing the highest influence of the particle and where the influence of that particle fades as you move further out from the center. 

A kernel function allows us to take a distance between two particles and work out the 'influence' of one particle on the other. An example of a simple kernel function would be a simple triangular kernel.

![triangle](https://github.com/user-attachments/assets/2dbfd6d2-9078-4580-a2e8-6f13d89f6ce4)

here is a 2D example, where the particle to compare with is at $x = 0$ and the x axis represents the distance from that particle. The y axis represents the quantity we want to approximate. A simple value we would want to approximate is density, where for each particle we accumulate a density value by adding higher values for closer neighbours and lower values for neighours further away. This kernel function would help compute this, resulting in a smoothing of the density. This is exactly how we can approximate a continuous field into discrete points effectively. Unlike scalar values (density), approximating forces like pressure requires us to use the gradient of the kernel. Pressure pushes particles away from each other, with a stronger force for particles closer together. There is also the viscosity force which uses the laplacian of the kernel (2nd order derivative) to gradually smooth out the velocity differences between particles. How I used kernels and more specifically which ones I used will be covered later as the triangular kernel unfortunately won't cut it. 

# **Navier-Stokes Equation**

The core equation: where we combine the conservation law of linear momentum with the constitutive relation for incompressible flow. 
- $\rho \frac{Dv}{Dt} = -\nabla \rho + \mu \nabla^2 v + \mathbf{f}_{ext}$
- $\rho$ - fluid density, $\mathbf{v}$ - velocity field, $p$ - pressure, $\mu$ - dynamic viscosity, $\mathbf{f}_{ext}$ - external forces

### A breakdown
- $\rho \frac{Dv}{Dt}$ - the acceleration ($a$), the equation is in the form $ma = F$, therefore we also multiply by the density
- $-\nabla \rho$ - represents the pressure force, with the negative gradient pointing towards decreasing pressure.
- $\mu \nabla^2 v$ - viscosity smoothing by using the laplacian of the velocity field, also multiplied by the viscosity constant
- $\mathbf{f}_{ext}$ - external forces such as gravity

# Implementation
## Setup
**Classes**
- Application
  - creating the window (GLFW)
  - handling user input
  - controlling the simulation loop
- Sim
  - storing and handling particle specific data
  - update function dispatches simulation compute shaders
- Grid
  - storing and handling neighbourhood grid data
  - build function dispatches neighbourhood compute shaders

classes from https://learnopengl.com/
- Camera
  - to move and look around the simulation
- Shader
  - compile, attach parameters, and run shaders

#### Particles
The first thing I had to think about were the data structures. I opted to use the **ShaderStorageBufferObject** (SSBO) available with OpenGL 4.3. They act similarly to **UniformBufferObjects** (UBO) except they can be much larger and allow for more functionality. Therefore, all the particle's attributes will be stored in SSBOs, following a **Structure-of-Arrays** (SoA) layout meaning each of the attributes will be stored within its own buffer (one for positions, velocities, densities...). The layout's signficance will become more apparent when I talk about compute shaders. More about buffer creation can be found in `Sim::initSSBO()`. 

**Particle Update**
As found in `Sim::update()`, the particles undergo several **compute shaders**. Compute shaders are GPU programs that are run for general purpose computations. It synergises very well with physical simulations and here it pairs with the rendering process for added benefit. The main idea is: instead of updating every particle one at a time, we update each particle at the same time. This introduces several potential issues, however if handled properly, can result in a much more efficient simulation. 

To enable a compute-shader driven update method, particles will be updated in several steps involving several compute shaders. This mainly exists to avoid race conditions, one of the potential issues with multi-threaded programming. I will re-compute the density for each particle near the start of the update cycle with a single compute shader called `updateDensity.comp`. Since each of the following compute shaders: `updateViscosity.comp`, `updatePressure.comp` require the updated density, it leads to a workflow like this: 

1. dispatch compute shader to assign new density values
2. impose a memory barrier for buffers
3. dispatch compute shader to assign new velocity based on the viscosity force (with the new density values)
4. impose a memory barrier for buffers
5. dispatch compute shader to compute the pressure force based on the new velocity
6. ...

where the memory barrier ensures all threads finish before moving on

#### Compute Shader
Each compute shader is written separately and compiled at the start of execution with the `Shader.h` class (thanks https://learnopengl.com/). They will all follow a pattern that goes along the lines of this: 

```C++
layout (local_size_x = 256) in     // number of threads for each thread_group


layout(std430, binding = 0) buffer PositionsSSBO {    // this compute shader will read/write to the positions SSBO
  vec4 positions[];
};


uniform float smoothing_length;     // essentially shader parameters that we set before dispatching


void main()     // shader will execute this function first just like in regular C++
{
    /* for updating density
        grab the thread_id as the particle index
        go through all this particle's neighbours and accumulate the density
        assign the density back to the density buffer */
}
```
#### NeighbourHood Search (Grid)

#### Rendering (OpenGL)




















