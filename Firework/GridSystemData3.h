#pragma once
#include <glm/vec3.hpp>

#include "BoundingBox.h"
#include "FaceCenteredGrid3.h"
#include "ScalarGrid3.h"
#include "Size3.h"

class GridSystemData3
{
public:
	GridSystemData3();

	virtual ~GridSystemData3();

	void resize(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& origin);

	Size3 resolution() const;

	glm::vec3<double> gridSpacing() const;

	glm::vec3<double> origin() const;

	BoundingBox3D boundingBox() const;

	size_t addScalarData(const ScalarGridBuilder3Ptr& builder, const glm::vec3<double>& initialVal = glm::vec3<double>());

	const FaceCenteredGrid3Ptr& velocity() const;

	const ScalarGrid3Ptr& scalarDataAt(size_t idx) const;

	const VectorGrid3Ptr& vectorDataAt(size_t idx) const;

	size_t numberOfScalarData() const;

	size_t numberOfVectorData() const;

private:
	FaceCenteredGrid3Ptr _velocity;
	std::vector<ScalarGrid3Ptr> _scalarDataList;
	std::vector<VectorGrid3Ptr> _vectorDataList;
};
