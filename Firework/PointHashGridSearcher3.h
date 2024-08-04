#pragma once
#include <functional>
#include <vector>
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"
#include "Size3.h"

typedef glm::vec3<double> vec3D;

class PointHashGridSearcher3
{
public:
	PointHashGridSearcher3(size_t resolutionX, size_t resolutionY, size_t resolutionZ, double gridSpacing, 
		unsigned int particleNum, GLuint bindingPoint);

	// build
	void forEachNearbyPoint(const glm::vec3& origin, double radius, const std::function<void(size_t, const glm::vec3&)>& callback) const;

	void build(GLuint particleNum);

	void fillNeighbourList();

private:

	size_t getHashKeyFromPosition(const glm::vec3& position) const;

	void getNearbyKeys(const glm::vec3& position, size_t* bucketIndices) const;

	// grid holds start/end index of neighboursSSBO for each grid. 
	GLuint binsSSBO, neighboursSSBO, particleIndicesSSBO, prefixSumSSBO, reorderedSSBO, tempBinOffsetSSBO;

	std::unique_ptr<Shader> countShader;
	std::unique_ptr<Shader> prefixShader;
	std::unique_ptr<Shader> reorderShader;

	double _gridSpacing = 1.0;
	Size3<int> _resolution = Size3<int>(25, 50, 25);
	GLuint resolution;
};
