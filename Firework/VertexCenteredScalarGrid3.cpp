#include "VertexCenteredScalarGrid3.h"

Size3 VertexCenteredScalarGrid3::dataSize() const
{
	if (resolution() != Size3(0, 0, 0))
		return resolution() + Size3(1, 1, 1);
	else
		return Size3(0, 0, 0);
}

glm::vec3<double> VertexCenteredScalarGrid3::dataOrigin() const
{
	return origin();
}
