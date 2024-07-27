#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"

#include <unordered_map>

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

	void addFirework(const glm::vec3& origin);

	GLuint getAliveCount();
	void resetAliveCount();

private:
	void initBuffers();
	void initShaders();

	std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
	GLuint VBO, VAO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, fluidSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	unsigned int particle_num, max_particles;
	float firework_lifetime;
	DrawElementsIndirectCommand cmd;
};
