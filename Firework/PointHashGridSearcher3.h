#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"

class PointHashGridSearcher3
{
public:
	PointHashGridSearcher3(GLuint resolutionX, GLuint resolutionY, GLuint resolutionZ, unsigned int particleNum, GLuint maxNeighbourNum);

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

	std::unique_ptr<Shader> countShader;
	// std::unique_ptr<Shader> pass1, pass2, pass3;
	std::unique_ptr<Shader> reorderShader;
	std::unique_ptr<Shader> generateNeighbourListShader;

	glm::vec3 resolutionVec;
	GLuint binCount;
	GLuint maxneighbourNum;
};
