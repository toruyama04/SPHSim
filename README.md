# SPH Sim

## Theory
1. SPH discretisation
We must be able to discretise spatial field quantities and spatial differential operators. Conceptually, we can say the exact value of a continuous field at any point in space can be computed by using the Dirac delta δ. So given a function $A$, our continous field, and our dirac delta distribution, by finding the convolution between them, we express the field value at location $\mathbf{x}$ as: $$A(x) = (A * \delta)(x) = \int A(\mathbf{x}')\delta (\mathbf{x} - \mathbf{x}') d\mathbf{x}'$$

The Dirac delta allows us to mathematically define perfect point sampling. However, since the Dirac delta is not a function but a mathematical distribution, and computers can't handle infinitely narrow sampling, we are unable to use $\delta$ directly for numerical simulations. Instead, we will replace $\delta$ with a continous approximation and discretise the resulting integral with particle sums. Our new kernel approximation takes the form: $$A(x) \approx (A * W)(x) = \int A(x')W(x-x',h)dv'$$ , where $h$ is the kernel's smoothing length. This parameter controls how far the influence of neighbouring points extend. 

