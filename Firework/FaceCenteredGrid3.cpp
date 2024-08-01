#include "FaceCenteredGrid3.h"


void FaceCenteredGrid3::onResize(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& origin,
	const glm::vec3<double>& initialValue)
{
	_dataU.resize(resolution + Size3(1, 0, 0), initialValue.x);
	_dataV.resize(resolution + Size3(0, 1, 0), initialValue.y);
	_dataW.resize(resolution + Size3(0, 0, 1), initialValue.z);
	_dataOriginU = origin + 0.5 * glm::vec3<double>(0.0, gridSpacing.y, gridSpacing.z);
	_dataOriginV = origin + 0.5 * glm::vec3<double>(gridSpacing.x, 0.0, gridSpacing.z);
	_dataOriginW = origin + 0.5 * glm::vec3<double>(gridSpacing.x, gridSpacing.y, 0.0);
}
