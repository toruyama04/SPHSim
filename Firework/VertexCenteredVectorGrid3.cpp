#include "VertexCenteredVectorGrid3.h"

Size3 VertexCenteredVectorGrid3::dataSize() const
{
	return resolution() + Size3(1, 1, 1);
}

glm::vec3<double> VertexCenteredVectorGrid3::dataOrigin() const
{
	return origin();
}
