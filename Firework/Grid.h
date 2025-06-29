#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"

class Grid
{
public:
	Grid(GLuint resolutionX, GLuint resolutionY, GLuint resolutionZ, GLuint particleNum, GLuint maxNeighbourNum);
	~Grid();
	void build(GLuint particleNum, float searchRadius);

private:

	GLuint particleNumPerBinSSBO;
	GLuint binIndexForParticleSSBO;
	GLuint prefixForBinReorderSSBO;
	GLuint particlesOrderedByBinSSBO;
	GLuint flatNeighboursSSBO;
	GLuint neighbourListSSBO;
	GLuint prefixIndexCounter;
	GLuint endIndexNeighbourSSBO;

	Shader* countShader;
	Shader* reorderShader;
	Shader* generateNeighbourListShader;

	glm::vec3 resolutionVec;
	GLuint binCount;
	GLuint maxneighbourNum;
};
