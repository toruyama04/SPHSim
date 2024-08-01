#include "CellCenteredScalarGrid3.h"


Size3 CellCenteredScalarGrid3::dataSize() const
{
	return resolution();
}

glm::vec3<double> CellCenteredScalarGrid3::dataOrigin() const
{
	return origin() + 0.5 * gridSpacing();
}

ScalarGrid3Ptr CellCenteredScalarGridBuilder3::build(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& gridOrigin, double initialVal) const
{
	return std::make_shared<CellCenteredScalarGrid3>(resolution, gridSpacing, gridOrigin, initialVal);
}
