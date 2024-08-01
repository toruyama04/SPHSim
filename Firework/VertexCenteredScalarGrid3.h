#pragma once
#include "ScalarGrid3.h"

class VertexCenteredScalarGrid3 final : public ScalarGrid3
{
public:
	VertexCenteredScalarGrid3();

	VertexCenteredScalarGrid3(const Size3& resolution, const glm::vec3<double>& gridSpacing = glm::vec3<double>(1.0, 1.0, 1.0),
		const glm::vec3<double>& origin = glm::vec3<double>(), double initialValue = 0.0);

	Size3 dataSize() const override;

	virtual glm::vec3<double> dataOrigin() const override;
};
