#pragma once

#include <glm/vec3.hpp>

template <typename T>
class BoundingBox<T, 3>
{
public:
	glm::vec3<T> lowerCorner;
	glm::vec3<T> upperCorner;

	BoundingBox();

	BoundingBox(const glm::vec3<T>& point1, const glm::vec3<T>& point2);



};


template <typename T>
using BoundingBox3 = BoundingBox<T>;

typedef BoundingBox3<float> BoundingBox3F;

typedef BoundingBox3<double> BoundingBox3D;

