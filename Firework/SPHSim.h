#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"

#include "PointHashGridSearcher3.h"

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

	Sim();
	~Sim();

	void render(const glm::mat4& view, const glm::mat4& projection);
	void update(float delta_time);

	void addSim(const glm::vec3& origin, const GLuint particle_num);
	void addParticleCube(const glm::vec3& origin, float spacing, int particlesPerSide);

	GLuint getAliveCount();
	void resetAliveCount(GLuint amount);

private:
	void initBuffers();
	void initShaders();

	unsigned int _max_particles;
	float sim_lifetime;

	// particleSystemData
	float _radius = 1.0f;
	float _mass;
	GLuint maxNeighbourNum = 150;
	float _targetDensity = 2.0f;

	std::unique_ptr<Shader> particleShader;
	std::unique_ptr<Shader> densityUpdate;
	std::unique_ptr<Shader> viscosityUpdate;
	std::unique_ptr<Shader> pressureCompute;
	std::unique_ptr<Shader> pressureUpdate;
	std::unique_ptr<Shader> timeIntegrations;
	std::unique_ptr<Shader> resetShader;
	std::unique_ptr<Shader> velocityIntermediate;

	std::unique_ptr<PointHashGridSearcher3> _neighbour_grids;

	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, forcesSSBO, densitiesSSBO, pressureSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	GLuint testSSBO;
	// GLuint particleTexture;
	DrawElementsIndirectCommand cmd;
};
