#include "ScalarGrid3.h"




ScalarGrid3::ScalarGrid3(GLuint bindingPoint)
{
	glGenBuffers(1, &_data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _data);
	glBufferData(GL_SHADER_STORAGE_BUFFER, resolution().x * resolution().y * resolution().z * sizeof(double), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, _data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


// getters do a mapping to get the data at a point

// gradient at any given point, interpolates from nearby locations
/*glm::vec3<double> ScalarGrid3::gradient(const glm::vec3<double>& x)
{
	std::array<
}*/