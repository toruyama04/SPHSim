#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "shader.h"
#include "Application.h"

#include <vector>

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
};

class Firework : public Application
{
public:
	Firework();
	Firework(unsigned int firework_num);
	~Firework();

	void run() override;

	void setDefaultValues();
	void initialiseBuffers(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocity);
	void updateParticles();
	void addShaders();

	template<typename T>
	void setupSSBO(unsigned int buf, std::vector<T>& type, int bindPoint, std::vector<T>& defaultValues)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * max_particles, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * max_particles, defaultValues.data());
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * particle_num, type.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, buf);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

private:
	GLuint VAO, VBO, EBO, SSBO[2], DIB, ACB, flags, floorVAO, floorVBO;
	unsigned int firework_num, particle_num, max_particles;
	float firework_lifetime;
	bool add_floor;
	DrawElementsIndirectCommand cmd;
};
