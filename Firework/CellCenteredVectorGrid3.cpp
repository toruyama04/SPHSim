#include "CellCenteredVectorGrid3.h"

Size3 CellCenteredVectorGrid3::dataSize() const
{
	return resolution();
}

glm::vec3<double> CellCenteredVectorGrid3::dataOrigin() const
{
	return origin() + 0.5 * gridSpacing();
}

