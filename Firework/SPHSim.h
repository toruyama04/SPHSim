#pragma once
#include <glad/glad.h>

#include <glm/vec4.hpp>

#include "Shader.h"
#include "Grid.h"

#include <vector>


class Sim
{
public:

	Sim(glm::vec3 origin, glm::vec3 gridExtent);
	~Sim();

	void render(const glm::mat4& view, const glm::mat4& projection);
	void update(float delta_time);

	void addParticleCube(const glm::vec3 origin, float spacing, GLuint particlesPerSide);

private:

	void initBuffers();
	void initSSBO();
	void initShaders();
	GLuint addBoundaryParticles(std::vector<glm::vec4>& positions, float spacing, int layers);

	glm::vec3 origin;
	glm::vec3 extents;

	// particleSystemData
	// radius is also the 'smoothing length' also referred to as 'h'
	float radius = 0.25f;
	float particleMass;
	GLuint fluidParticleNum;
	GLuint totalParticles;
	GLuint boundaryParticleNum;
	GLuint maxNeighbourNum = 50;
	float targetDensity = 1000.0f;
	float sphereRadius = 0.03f;

	Shader* particleShader;
	Shader* densityUpdate;
	Shader* viscosityUpdate;
	Shader* pressureUpdate;
	Shader* timeIntegrations;
	Shader* positionPredict;

	Grid* neighbourGrid;

	GLuint VBO, VAO;
	GLuint positionsSSBO;
	GLuint velocitiesSSBO;
	GLuint forcesSSBO;
	GLuint densitiesSSBO;
	GLuint predPositionsSSBO;
	GLuint colourSSBO;

};
