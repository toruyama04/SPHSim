#pragma once

#include "ScalarGrid3.h"
#include "FaceCenteredGrid3.h"
#include "CollocatedVectorGrid3.h"

class GridBackwardEulerDiffusionSolver3
{
public:
	GridBackwardEulerDiffusionSolver3();

	~GridBackwardEulerDiffusionSolver3();

	// add boundaries?
	void solve(const ScalarGrid3& source, double diffusionCoefficient, double delta_time,
		ScalarGrid3* dest);

	void solve(const CollocatedVectorGrid3& source, double diffusionCoefficient, double delta_time,
		CollocatedVectorGrid3* dest);

	void solve(const FaceCenteredGrid3& source, double diffusionCoefficient, double delta_time,
		FaceCenteredGrid3* dest);

private:
	// boundaryType, fdmlinearSystem, 3D vector _markers
};

typedef std::shared_ptr<GridBackwardEulerDiffusionSolver3> GridBackwardEulerDiffusionSolver3Ptr;
