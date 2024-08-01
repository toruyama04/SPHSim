#pragma once
#include "CollocatedVectorGrid3.h"

class CellCenteredVectorGrid3 final : public CollocatedVectorGrid3
{
public:
	CellCenteredVectorGrid3();

	Size3 dataSize() const override;

	glm::vec3<double> dataOrigin() const override;
};
