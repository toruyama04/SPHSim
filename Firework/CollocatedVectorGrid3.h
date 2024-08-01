#pragma once
#include "VectorGrid3.h"

class CollocatedVectorGrid3 : public VectorGrid3
{
public:
	CollocatedVectorGrid3();

	virtual ~CollocatedVectorGrid3();

	virtual Size3 dataSize() const = 0;

	virtual glm::vec3<double> dataOrigin() const = 0;

	const glm::vec3<double>& operator()(size_t i, size_t j, size_t k) const;

	glm::vec3<double>& operator()(size_t i, size_t j, size_t k);

private:
	std::vector<std::vector<std::vector<glm::vec3>>> _data;
};
