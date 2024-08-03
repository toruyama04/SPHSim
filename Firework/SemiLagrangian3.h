#pragma once

#include "ScalarGrid3.h"
#include "CollocatedVectorGrid3.h"
#include "FaceCenteredGrid3.h"

class SemiLagrangian3
{
public:
	SemiLagrangian3();

	virtual ~SemiLagrangian3();

	void advect(const ScalarGrid3& input, const VectorField3& flow, double delta_time,
		ScalarGrid3* output);

	void advect(const CollocatedVectorGrid3& input, const VectorField3& flow, double delta_time,
		CollocatedVectorGrid3* output);

	void advect(const FaceCenteredGrid3& input, const VectorField3& flow, double delta_time,
		FaceCenteredGrid3* output);

private:
	glm::vec3<double> backTrace(const VectorField3& flow, double delta_time, double h,
		const glm::vec3<double> pt0);
};

typedef std::shared_ptr<SemiLagrangian3> SemiLagrangian3Ptr;
