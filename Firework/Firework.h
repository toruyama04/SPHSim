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

	void init();
	void render(const glm::mat4& view, const glm::mat4& projection);
	void update(float delta_time);

	void addFirework(const glm::vec3& origin);
	void initialiseBuffers();

	GLuint getAliveCount() const;
	void resetAliveCount();

private:
	void initBuffers();
	void initShaders();

	std::unordered_map<std::string, Shader*> shaders;
	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	unsigned int particle_num, max_particles;
	float firework_lifetime;
	DrawElementsIndirectCommand cmd;
};
