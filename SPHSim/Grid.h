#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"

class Grid
{
public:
	Grid(glm::vec3 gridOrigin, glm::vec3 extents, GLuint totalParticles,
		GLuint fluid, GLuint maxNeighbourNum, float gridSpacing);
	~Grid();
	void build(float searchRadius);

private:

	GLuint particleNumPerBinSSBO;
	GLuint particlesOrderedByBinSSBO;
	GLuint prefixForBinReorderSSBO;
	GLuint binIndexForParticleSSBO;
	GLuint flatNeighboursSSBO;
	GLuint neighbourListSSBO;
	GLuint prefixIndexCounterSSBO;
	GLuint endIndexNeighbourSSBO;
	GLuint neighbourBinOffsetSSBO;

	Shader* countShader;
	Shader* reorderShader;
	Shader* buildNeighbourListShader;
	Shader* computePrefixSumShader;

	glm::vec3 resolutionVec;
	GLuint binCount;
	GLuint maxNeighbourNum;
	float gridSpacing;
	glm::vec3 gridOrigin;
	GLuint totalParticles;
	GLuint fluidParticleNum;
	GLuint boundaryParticleNum;
};
