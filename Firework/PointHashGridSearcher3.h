#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Shader.h"

class PointHashGridSearcher3
{
public:
	PointHashGridSearcher3(GLuint resolutionX, GLuint resolutionY, GLuint resolutionZ, double gridSpacing,
		unsigned int particleNum);

	void build(GLuint particleNum);

private:

	GLuint particleNumPerBinSSBO, binIndexForParticleSSBO;
	GLuint prefixForBinReorderSSBO, particlesOrderedByBinSSBO;
	GLuint flatNeighboursSSBO, neighbourListSSBO;
	GLuint neighbourOffsetSSBO, prefixIndexCounter;
	GLuint groupSumSSBO, groupPrefixSumSSBO;
	GLuint endIndexNeighbourSSBO;

	std::unique_ptr<Shader> countShader;
	std::unique_ptr<Shader> pass1, pass2, pass3;
	std::unique_ptr<Shader> reorderShader;
	std::unique_ptr<Shader> generateNeighbourListShader;

	glm::uvec3 _resolution;
	GLuint resolution;
};
