#pragma once
#include <functional>
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "BoundingBox.h"
#include "Size3.h"
#include <GLFW/glfw3.h>


class Grid3
{
public:
	typedef std::function<glm::vec3(size_t, size_t, size_t)> DataPositionFunc;

	Grid3();

	virtual ~Grid3();

	const Size3& resolution() const;

	const glm::vec3<double>& origin() const;

	const glm::vec3<double>& gridSpacing() const;

	const BoundingBox3D& boundingBox() const;

	DataPositionFunc cellCenterPosition() const;

	// forEachCellIndex() -> invokes function for each grid cell -> compute dispatch

private:
	Size3 _resolution;

	glm::vec3<double> _gridSpacing = glm::vec3<double>(1, 1, 1);

	glm::vec3<double> _origin;

	BoundingBox3D _boundingBox = BoundingBox3D(glm::vec3<double>(), glm::vec3<double>());
};
