#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"

#include <unordered_map>
#include "PointHashGridSearcher3.h"

class Firework
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

	Firework();
	~Firework();

	void render(const glm::mat4& view, const glm::mat4& projection);


	void update(float delta_time);

	void beginAdvanceTimeStep(GLuint count);
	void accumulateNonPressureForce(GLuint count);
	void accumulatePressureForce(GLuint count);
	void timeIntegration(GLuint count, float delta_time);

	void addFirework(const glm::vec3& origin);
	void addParticleCube(const glm::vec3& origin, float spacing, int particlesPerSide);

	GLuint getAliveCount();
	void resetAliveCount(GLuint amount);

private:
	void initBuffers();
	void initShaders();

	unsigned int _particle_num, _max_particles, _firework_num;
	float firework_lifetime;

	// particleSystemData
	float _radius = 1.0;
	float _mass = 1e-3;
	glm::vec3 _gravity = glm::vec3(0.0f, 2.0f, 0.0f);
	float _targetDensity = 800.0;
	// double _targetSpacing = 0.1;
	// double pseudoViscosityCoefficient = 10.0;
	// double _kernelRadiusOverTargetSpacing = 1.8;
	float _eosExponent = 7.0;
	float _negativePressureScale = 0.0;

	std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
	std::unique_ptr<PointHashGridSearcher3> _neighbour_grids;

	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, forcesSSBO, densitiesSSBO, pressureSSBO;
	GLuint newPositionSSBO, newVelocitySSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	// GLuint particleTexture;
	DrawElementsIndirectCommand cmd;
};
