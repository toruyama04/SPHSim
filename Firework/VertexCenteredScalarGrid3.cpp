#include "VertexCenteredScalarGrid3.h"

Size3 VertexCenteredScalarGrid3::dataSize() const
{
	return resolution() + Size3(1, 1, 1);
}

glm::vec3<double> VertexCenteredScalarGrid3::dataOrigin() const
{
	return origin();
}
