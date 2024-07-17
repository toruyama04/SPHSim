#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"
#include "Application.h"

#include <vector>
#include <unordered_map>

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
};

class Firework
{
public:
	Firework();
	~Firework();
    Firework(Firework&& other) noexcept
        : shaders(std::move(other.shaders)), VBO(other.VBO), VAO(other.VAO), EBO(other.EBO),
        positionsSSBO(other.positionsSSBO), velocitiesSSBO(other.velocitiesSSBO),
        aliveFlagSSBO(other.aliveFlagSSBO), atomicCounterBuffer(other.atomicCounterBuffer),
        drawIndirectBuffer(other.drawIndirectBuffer), particle_num(other.particle_num),
        max_particles(other.max_particles), firework_lifetime(other.firework_lifetime),
        origin_pos(std::move(other.origin_pos)), cmd(std::move(other.cmd)) {
        other.VBO = 0;
        other.VAO = 0;
        other.EBO = 0;
        other.positionsSSBO = 0;
        other.velocitiesSSBO = 0;
        other.aliveFlagSSBO = 0;
        other.atomicCounterBuffer = 0;
        other.drawIndirectBuffer = 0;
    }
 
	void init();
	void render(const glm::mat4& view, const glm::mat4& projection);
	void update(const float delta_time);

	void addFirework(const glm::vec3& origin);
	void initialiseBuffers();

	GLuint getAliveCount();
	void resetAliveCount();

private:
	void initBuffers();
	void initShaders();
	void addShader(const std::string& shader_name, Shader* shader);

	std::unordered_map<std::string, Shader*> shaders;
	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	unsigned int particle_num, max_particles;
	float firework_lifetime;
	glm::vec3 origin_pos;
	DrawElementsIndirectCommand cmd;
};
