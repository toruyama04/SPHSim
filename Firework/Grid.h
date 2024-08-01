#pragma once
#include <functional>
#include <glm/vec3.hpp>

#include "BoundingBox.h"
#include "Size3.h"


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
};
