#include "Firework.h"
#include <glad/glad.h>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "utils.h"

Firework::Firework() : Firework(1)
{
}

Firework::~Firework()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &DIB);
    glDeleteBuffers(1, &ACB);
    glDeleteBuffers(1, &flags);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(2, SSBO);
}

Firework::Firework(unsigned int firework_num) : firework_num{firework_num}
{
    firework_lifetime = 3.0f;
    add_floor = true;
    particle_num = 8192;
    max_particles = 65536;

	glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &ACB);
    glGenBuffers(2, SSBO);
    glGenBuffers(1, &DIB);
    glGenBuffers(1, &flags);

    setDefaultValues();
    addShaders();
}

void Firework::addShaders()
{
    addShader("particleShader", "shaders/particle.vert", "shaders/particle.frag");
    addShader("computeShaderUpdate", "shaders/particleUpdate.comp");
    addShader("computeShaderPrefix", "shaders/particlePrefix.comp");
    addShader("computeShaderTrail", "shaders/particleTrail.comp");
    addShader("computeShaderReorder", "shaders/particleReorder.comp");
    addShader("floorShader", "shaders/floor.vert", "shaders/floor.frag");
}

void Firework::run()
{
	try
	{
		shaders["floorShader"]->use();
	    shaders["floorShader"]->setVec3("colour", glm::vec3(0.5f, 0.5f, 0.5f));

	    while (!glfwWindowShouldClose(window))
	    {
	        float currentFrame = static_cast<float>(glfwGetTime());
	        deltaTime = currentFrame - lastFrame;
	        lastFrame = currentFrame;

	        processInput(window);

	        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	        shaders["particleShader"]->use();
	        glm::mat4 view = camera->GetViewMatrix();
	        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100.0f);
	        glm::mat4 model = glm::mat4(1.0f);
	        shaders["particleShader"]->setMat4("model", model);
	        shaders["particleShader"]->setMat4("view", view);
	        shaders["particleShader"]->setMat4("projection", projection);

	        shaders["floorShader"]->use();
	        model = glm::mat4(1.0f);
	        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
	        model = glm::scale(model, glm::vec3(2.0f));
	        shaders["floorShader"]->setMat4("model", model);
	        shaders["floorShader"]->setMat4("view", view);
	        shaders["floorShader"]->setMat4("projection", projection);
	        updateParticles();

	        displayFPS();
	        glfwSwapBuffers(window);
	        glfwPollEvents();
	    }		
	}
	catch (const std::out_of_range& e)
	{
        std::cerr << "Shader not found: " << e.what() << "\n";
	}
    
}

void Firework::updateParticles()
{
	try
	{
	    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
	    GLuint initial = 0;
	    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &initial);

	    // update particle data
	    shaders["compShaderUpdate"]->use();
	    shaders["compShaderUpdate"]->setFloat("deltaTime", deltaTime);
	    shaders["compShaderUpdate"]->setVec3("grav", glm::vec3(0.0f, -2.81f, 0.0));
	    shaders["compShaderUpdate"]->setFloat("minVelocity", 0.1f);
	    glDispatchCompute(max_particles / 256, 1, 1);
	    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	    // prefix sum
	    shaders["computeShaderPrefix"]->use();
	    shaders["computeShaderPrefix"]->setInt("maxParticle", max_particles);
	    glDispatchCompute(max_particles / 1024, 1, 1);
	    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	    // reorder
	    shaders["computeShaderReorder"]->use();
	    glDispatchCompute(max_particles / 1024, 1, 1);
	    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	    // add trail particles
	    shaders["computeShaderTrail"]->use();
	    shaders["computeShaderTrail"]->setFloat("trailRate", 0.9f);
	    shaders["computeShaderTrail"]->setUInt("maxParticle", max_particles);
	    glDispatchCompute(particle_num / 256, 1, 1);
	    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
	    GLuint* num = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
	    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	    cmd.instanceCount = num[0];
	    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, DIB);
	    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_DRAW);

	    glBindVertexArray(VAO);
	    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
	    glBindVertexArray(0);

	    glBindVertexArray(floorVAO);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	    glBindVertexArray(0);		
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << "Shader not found: " << e.what() << "\n";
	}


}

void Firework::setDefaultValues()
{
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> velocity;
	for (unsigned int i = 0; i < firework_num; ++i)
	{
		for (unsigned int j = 0; j < particle_num; ++j)
		{
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(2.0f, 5.0f);
            velocity.emplace_back(speed * sin(phi) * cos(theta), speed * sin(phi) * sin(theta), speed * cos(phi), firework_lifetime);
            positions.emplace_back(fireworkPos[i].x, fireworkPos[i].y, fireworkPos[i].z, firework_lifetime);
		}
	}
    initialiseBuffers(positions, velocity);
}

void Firework::initialiseBuffers(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocity)
{
    glBindVertexArray(VAO);
    cmd = { 6, 1, 0, 0, 0 };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ACB);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    std::vector<glm::vec4> default_values(max_particles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    setupSSBO<glm::vec4>(SSBO[0], positions, 0, default_values);
    setupSSBO<glm::vec4>(SSBO[1], velocity, 1, default_values);
    std::vector<int> flag_default(max_particles, 0);
    std::vector<int> flag_firework(particle_num, 1);
    setupSSBO<int>(flags, flag_firework, 2, flag_default);

    if (add_floor)
    {
        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);

        glBindVertexArray(floorVAO);

        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, floorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}




