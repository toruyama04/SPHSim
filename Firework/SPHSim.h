#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"
#include "Grid.h"

#include <vector>

class Sim
{
public:
	struct DrawElementsIndirectCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLint baseVertex;
		GLuint baseInstance;
	};

	Sim(glm::vec3 origin, glm::vec3 gridExtent);
	~Sim();

	void render(const glm::mat4& view, const glm::mat4& projection);
	void update(float delta_time);

	//void addSim(const glm::vec3& origin, const GLuint particle_num);
	void addParticleCube(const glm::vec3 origin, float spacing, int particlesPerSide);
	GLuint addBoundaryParticles(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocities,
		float spacing, int layers);

	GLuint getAliveCount();
	void resetAliveCount(GLuint amount);

private:
	void initBuffers();
	void initShaders();

	unsigned int _max_particles;
	float sim_lifetime;
	glm::vec3 origin;
	glm::vec3 extents;

	// particleSystemData
	// radius is also the 'smoothing length' also referred to as 'h'
	float _radius = 0.2f;
	float _mass;
	GLuint fluidParticleNum;
	GLuint totalParticles;
	GLuint boundaryParticleNum;
	GLuint maxNeighbourNum = 50;
	float _targetDensity = 1000.0f;

	Shader* particleShader;
	Shader* densityUpdate;
	Shader* viscosityUpdate;
	Shader* pressureCompute;
	Shader* pressureUpdate;
	Shader* timeIntegrations;
	Shader* resetShader;
	Shader* velocityIntermediate;

	Grid* _neighbour_grids;

	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, forcesSSBO, densitiesSSBO, pressureSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	GLuint predictedVelocitySSBO;
	// GLuint particleTexture;
	DrawElementsIndirectCommand cmd;
};
