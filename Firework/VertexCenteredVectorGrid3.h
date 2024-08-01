#pragma once
#include "CollocatedVectorGrid3.h"

class VertexCenteredVectorGrid3 : public CollocatedVectorGrid3
{
public:
	VertexCenteredVectorGrid3();

	Size3 dataSize() const override;

	glm::vec3<double> dataOrigin() const override;
};
