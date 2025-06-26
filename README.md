# SPH Sim

based on: https://sph-tutorial.physics-simulation.org/pdf/SPH_Tutorial.pdf#cite.IABT11
 
## SPH discretisation
We must be able to discretise spatial field quantities and spatial differential operators. Conceptually, we can say the exact value of a continuous field at any point in space can be computed by using the Dirac delta δ. So given a function $A$, our continuous field, and our Dirac delta distribution, by finding the convolution between them, we express the field value at location $\mathbf{x}$ as: $$A(x) = (A * \delta)(x) = \int A(\mathbf{x}')\delta (\mathbf{x} - \mathbf{x}') d\mathbf{x}'$$
The Dirac delta allows us to mathematically define perfect point sampling. However, since the Dirac delta is not a function but a mathematical distribution, and computers can't handle infinitely narrow sampling, we are unable to use $\delta$ directly for numerical simulations. Instead, we will replace $\delta$ with a continuous approximation and discretise the resulting integral with particle sums. Our new kernel approximation takes the form: $$A(x) \approx (A * W)(x) = \int A(x')W(x-x',h)dv'$$ where $h$ is the kernel's smoothing length. This parameter controls how far the influence of neighbouring points extend. We will impose some conditions to the smoothing kernel:

1. Normalisation condition:
	- $\int_{\mathbb{R} ^d}W(\mathbf{r}',h)dv' = 1$ 
	- must integrate to 1 over all space, preventing artificial amplification/damping of field values
2. Dirac-$\delta$ condition:
	- $\displaystyle \lim\limits_{h' \to 0} W(\mathbf{r}, h') = \delta(\mathbf{r})$
	- as $h$ tends to 0, the kernel must approach the Dirac delta.
3. Positivity condition:
	- $W(r,h) \ge 0$
	- the kernel must be non-negative everywhere, ensuring physical stability
	- not strongly required, however negative values may lead to unphysical results
4. Symmetry condition:
	- $W(r,h) = W(-r,h)$
	- the kernel must be radially symmetric
5. Compact Support Condition:
	- $W(\mathbf{r}, h) = 0 \quad \text{for } \lVert \mathbf{r} \rVert \geq \hbar$, where $\forall \mathbf{r} \in \mathbb{R}^d, h \in \mathbb{R}^+$, where $\bar{h}$ denotes the support radius of the kernel function
	- the kernel must be exactly zero beyond radius $h$
6. Should be twice continuously differentiable to enable a consistent discretisation of second-order partial differential equations (PDEs). 

## Cubic Spline Kernel

![[Pasted image 20250618224532.png]]
- with $q = \frac{1}{h}||r||$
- we have different normalisation factors depending on the dimension:
	- $\sigma_1 = \frac{4}{3h}$, $\sigma_2 = \frac{40}{7\pi h^2}$, $\sigma_3 = \frac{8}{\pi h^3}$ 
- this specific cubic spline kernel has the property where its smoothing length is identical to the kernel support radius. 

Discretisation 
$$\langle A(\mathbf{x}_i) \rangle = \sum_{j \in \mathcal{F}} A_j \frac{m_j}{\rho_j} W_{ij}$$
Mass Density Estimation
$$\rho_i = \sum_{j} m_jW_{ij}$$
Gradient of continuous field
$$\nabla A_i \approx \sum_{j} A_j \frac{m_j}{\rho_j}\nabla W_{ij}$$
Laplace of continuous field

## Governing Equations
Continuity Equation
$\frac{D\rho}{Dt} = -\rho (\nabla \cdot v)$
- describes the evolution of an object's mass density $\rho$ over time, with $\frac{D(\cdot)}{Dt}$ is the material derivative
$\frac{D\rho}{Dt} = 0 \Leftrightarrow \nabla \cdot v = 0$
- when divergence of velocity is 0, then material derivative of density should be 0
- when divergence of velocity is less than 0, particles are converging and density increases

Conservation Law of Linear Momentum
$\rho \frac{D^2 \mathbf{x}}{Dt^2} = \nabla \cdot \mathbf{T} + \mathbf{f}_{ext}$
- where, $\mathbf{T}$ denotes the stress tensor

Navier-Stokes Equation
constitutive relation for incompressible flow
$\Upsilon = -\rho 1 + \mu (\nabla v + \nabla v^T))$
- describes how internal stresses relate to fluid motion
- here $\rho$ denotes pressure, $\mu$ denotes viscosity 
- if we require incompressibility, we should interpret pressure as a Lagrange multiplier such that the constraint on the continuity equation is fulfilled. If not required, we can complete the constitutive relation by a 'state equation' 
$\rho \frac{Dv}{Dt} = -\nabla \rho + \mu \nabla^2 v + \mathbf{f}_{ext}$
- by plugging the conservation law of linear momentum to the constitutive relation for incompressible flow, we arrive at the Navier-Stokes equation

## Operator Splitting
As an example of an algorithm that splits the fluid solver into subproblems, lets consider an algorithm to solve for a low-viscous fluid with a strong enforcement of the incompressibility constraint.
1. Update $v$ by solving $\frac{Dv}{Dt} = \mathbf{v}\nabla^2v + \frac{1}{\rho}\mathbf{f}_{ext}$
2. determine $\nabla \rho$ by enforcing $\frac{D\rho}{Dt} = 0$
3. update $v$ by solving $\frac{Dv}{Dt} = -\frac{1}{\rho}\nabla \rho$
4. update $x$ by solving $\frac{Dx}{Dt} = v$
Where $\mathbf{v} = \frac{\mu}{\rho}$ denotes the kinematic viscosity

## Courant-Friedrichs-Lewy 
A condition required for solving the convergence of numerical solvers for differential equations, providing an upper bound for the time step width
$$\Delta t \le \lambda \frac{\bar{h}}{||v^{max}||}$$
where $\bar{h}$, $v^{max}$, and $\lambda$ denote the particle size, the maximum velocity of a particle, and a user-defined scaling parameter. We typically want the simulation to satisfy this condition

## Simple Fluid Simulator
- state-equation based simulator for weakly compressible fluids with operator splitting using SPH and symplectic Euler integration. 
- boundary conditions aren't specifically detailed

for all *particle* $i$ do:
	Reconstruct density $\rho_i$ at $x_i$ with $\rho_i = \sum_{j} m_jW_{ij}$
for all *particle* $i$ do:
	Compute $\mathbf{F}^{viscosity}_i = m_i \mathbf{v}\nabla^2 v_i$ using $\nabla^2 A_i = -\sum_j \frac{m_j}{\rho_j}A_{ij}\frac{2||\nabla_iW_{ij}||}{||r_{ij}||}$
	$v^*_i = v_i + \frac{\Delta t}{m_i}(\mathbf{F}^{viscosity}_i + \mathbf{F}^{ext}_i)$
for all *particle* $i$ do:
	Compute $\mathbf{F}^{pressure}_{i} = -\frac{1}{\rho}\nabla \rho$ using state equation and $\rho_i \sum_{j}m_j(\frac{A_i}{\rho^2_i}+\frac{A_j}{\rho^2_j})\nabla_i W_{ij}$
for all *particle* $i$ do:
	$v_i(t + \Delta t) = v^*_i + \frac{\Delta t}{m_i} \mathbf{F}^{pressure}_i$
	$x_i(t+\Delta t) = x_i + \Delta t v_i(t+\Delta t)$

## Neighbourhood Search
The algorithm requires going through each particle for each particle giving us a total runtime complexity of $O(n^2)$. If, instead, we maintain a list of neighbours for each particle such that it only contains particles that are within the kernel support radius, it greatly reduces the number of particles to iterate over. This reduces the runtime complexity to just $O(mn)$, where $m$ is the maximum number of neighbouring particles. And with $m$ usually being bounded, we reduce the complexity to just a linear runtime of $O(n)$

Compact Hashing
- A naïve approach to the neighbourhood search results in a quadratic computational complexity. Instead we can opt for a grid-based approach called compact hashing. 
- we split the area into grids with the size being the kernel support radius. for a given particle, we need only to search in the grid cells that neighbour itself and its own cell. 
- to reduce the memory constraints of storing each cells potential particles, we instead utilise a sparse representation of the grid, storing only the populated cells. 
	- we can use a hash map with the hash for a given index $(i,j,k)$ being $[p_1 i)\ XOR\ (p_2 j)\ XOR\ (p_3 k)] mod \ m$
	- where $p_1 = 73856093$, $p_2 = 19349663$, $p_3 =83492791$ 
	- we can reduce the frequency of hash collisions by increasing the hash table size.
	- However, this approach may not lead to a high cache-hit rate as cells physically close to each other, may not necessarily be close in memory. 

## Pressure Solvers
Computing Volume deviation
**SESPH (State Equation SPH)**
- through density: fluid volume oscillates due to an over-correction of the pressure acceleration
- examples: $p_i = k(\frac{\rho_i}{\rho^0} - 1)$, $p_i = k(\rho_i - \rho^0)$, $p_i = k_1((\frac{\rho_i}{\rho^0})^{k_2} - 1)$
- used for weakly compressible or compressible fluid simulations
- the stiffness constant in the state equation governs the density deviation. larger values result in smaller deviations and require smaller time steps. smaller values lead to larger density deviations, and consequently less realistic simulations

Pressure Poisson Equation (PPE)
- through velocity divergence: $\frac{D\rho}{Dt} = -\rho \nabla \cdot v$
	- results in a drift in the fluid volume, typically a volume loss

**IISPH (Implicit Incompressible SPH)**
$$\Delta t^2 \nabla^2 p_i = \rho^0 - \rho^*_i$$
where $\Delta t^2 \nabla^2 p_i$ = $\Delta t^2 \sum_{j} m_j (\mathbf{a}^P_i - \mathbf{a}^P_i) \cdot \nabla W_{ij}$
and $\mathbf{a}^P_i = -\frac{1}{\rho_i} \nabla p_i = -\sum_j m_j (\frac{p_i}{\rho^2_i} + \frac{p_j}{\rho^2_j}) \nabla W_{ij}$
then introducing the velocity change due to pressure acceleration can be written as
$$\Delta t \sum_j m_j(v^P_i - v^P_j) \cdot \nabla W_{ij} = \Delta t \rho_i \nabla \cdot v^P_i = \rho^0  - \rho^*_i$$
where $\rho^0$ is the rest density and $\rho^*$ is the predicted density
We can solve the pressure by recognising the equation forms a system $\mathbf{Ap} = \mathbf{s}$
- Relaxed Jacobi Scheme
	- $p_i^{(l+1)} = max(p_i^{(l)} + \frac{\mathbf{\omega}}{a_{ii}}(s_i - (\mathbf{Ap}^{(l)})_i),0)$
	- where $l$ is the iteration and $\omega$ is the relaxation coefficient, typically set to 0.5

$$a_{ii} = -\Delta t^2 \sum_j m_j(\sum_j \frac{m_j}{\rho^2_i} \nabla W_{ij}) \cdot \nabla W_{ij} - \Delta t^2 \sum_j m_j (\frac{m_i}{\rho^2_i} \nabla W_{ij}) \cdot W_{ij}$$
- stop criterion for Relaxed Jacobi Scheme is usually based on the predicated relative density error. 

for all *particle* $i$ do:
	compute diagonal element $a_{ii}$
	compute source term $s_i$ with $\rho^0 - \rho^*_i = \rho^0 - \rho_i - \Delta t \sum_j m_j(v^*_i - v^*_j) \cdot \nabla W_{ij}$
	initialise pressure $\rho^{(0)}_i = 0$
$l = 0$
repeat
	for all *particle* $i$ do
		compute pressure acceleration $(\mathbf{a}^P_i)^{(l)}$
	for all *particle* $i$ do
		compute Laplacian $(\mathbf{Ap}^{(l)})_i$ 
		update pressure $\rho^{(l+1)}_i$ 
	$l = l+1$
until $\rho^{avg_error}_i < 0.1$%

**PCISPH (predictive-corrective SPH)**
we avoid a linear system by making simplifications and approximations such that each pressure value $p_i$ can be computed from one equation

