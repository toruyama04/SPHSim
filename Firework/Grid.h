#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"

class Grid
{
public:
	Grid(glm::vec3 gridOrigin, glm::vec3 extents, GLuint particleNum, 
		GLuint maxNeighbourNum, float gridSpacing);
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
	Shader* prefixSumShader;

	glm::vec3 resolutionVec;
	GLuint binCount;
	GLuint maxNeighbourNum;
	float gridSpacing;
	glm::vec3 gridOrigin;
};
