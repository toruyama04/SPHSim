#pragma once
#include "ScalarGrid3.h"

class CellCenteredScalarGrid3 final  : public ScalarGrid3
{
public:
	CellCenteredScalarGrid3();

	CellCenteredScalarGrid3(const Size3& resolution, const glm::vec3<double>& gridSpacing = glm::vec3<double>(1.0, 1.0, 1.0),
		const glm::vec3<double>& origin = glm::vec3<double>(), double initialValue = 0.0);

	Size3 dataSize() const override;

	glm::vec3<double> dataOrigin() const override;
};

class CellCenteredScalarGridBuilder3 final : public ScalarGridBuilder3
{
public:
	CellCenteredScalarGridBuilder3();

	ScalarGrid3Ptr build(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& gridOrigin, double initialVal) const override;
};
