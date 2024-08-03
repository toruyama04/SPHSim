#include "VertexCenteredVectorGrid3.h"

Size3 VertexCenteredVectorGrid3::dataSize() const
{
	if (resolution() != Size3(0, 0, 0))
		return resolution() + Size3(1, 1, 1);
	else
		return Size3(0, 0, 0);
}

glm::vec3<double> VertexCenteredVectorGrid3::dataOrigin() const
{
	return origin();
}
