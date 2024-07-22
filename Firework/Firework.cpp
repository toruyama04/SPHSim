#include "Firework.h"
#include <glad/glad.h>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include "utils.h"

Firework::Firework()
{
    firework_lifetime = 3.0f;
    particle_num = 8192;
    max_particles = 204800;

	initBuffers();
	initShaders();
}

Firework::~Firework()
{
    for (auto const& pair : shaders)
    {
        delete pair.second;
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &positionsSSBO);
    glDeleteBuffers(1, &velocitiesSSBO);
    glDeleteBuffers(1, &aliveFlagSSBO);
    glDeleteBuffers(1, &atomicCounterBuffer);
    glDeleteBuffers(1, &drawIndirectBuffer);
}

void Firework::initShaders()
{
    shaders["particleShader"] = new Shader("shaders/particle.vert", "shaders/particle.frag");
    shaders["computeShaderUpdate"] = new Shader("shaders/particleUpdate.comp");
    shaders["computeShaderPrefix"] = new Shader("shaders/particlePrefix.comp");
    shaders["computeShaderTrail"] = new Shader("shaders/particleTrail.comp");
    shaders["computeShaderReorder"] = new Shader("shaders/particleReorder.comp");
}

void Firework::initBuffers()
{
    // initialising firework VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // initialising particle vertices buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // initialising indices element buffer
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    // initialising alive count to 0 for atomic counter buffer
    glGenBuffers(1, &atomicCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // initialising positions SSBO with default values
    glGenBuffers(1, &positionsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    std::vector default_values(max_particles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising velocity SSBO with default values
    glGenBuffers(1, &velocitiesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising alive flag SSBO with default values
    glGenBuffers(1, &aliveFlagSSBO);
    std::vector<int> flag_default(max_particles, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, aliveFlagSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * max_particles, flag_default.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, aliveFlagSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialising indirect buffer
    glGenBuffers(1, &drawIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    cmd = { 6, 1, 0, 0, 0 };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	/*if (add_floor)
    {
        glGenVertexArrays(1, &floorVAO);
        glGenBuffers(1, &floorVBO);

        glBindVertexArray(floorVAO);

        glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, floorVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }*/
}

void Firework::render(const glm::mat4& view, const glm::mat4& projection)
{
    GLuint count = getAliveCount();
    cmd = { 6, count, 0, 0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_DRAW);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
	/*shaders["floorShader"]->use();
    shaders["floorShader"]->setVec3("colour", glm::vec3(0.5f, 0.5f, 0.5f));*/
    glBindVertexArray(VAO);
    shaders["particleShader"]->use();
	glm::mat4 model = glm::mat4(1.0f);
    shaders["particleShader"]->setMat4("model", model);
    shaders["particleShader"]->setMat4("view", view);
    shaders["particleShader"]->setMat4("projection", projection);

    if (count > 0)
    {
		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    }
glBindVertexArray(0);

    /*shaders["floorShader"]->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.0f));
    shaders["floorShader"]->setMat4("model", model);
    shaders["floorShader"]->setMat4("view", view);
    shaders["floorShader"]->setMat4("projection", projection);*/
}

void Firework::update(float delta_time)
{
    resetAliveCount();

    // update particle data
    shaders["computeShaderUpdate"]->use();
    shaders["computeShaderUpdate"]->setFloat("deltaTime", delta_time);
    shaders["computeShaderUpdate"]->setVec3("grav", glm::vec3(0.0f, -2.81f, 0.0));
    shaders["computeShaderUpdate"]->setFloat("minVelocity", 0.1f);
    glDispatchCompute(max_particles / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // prefix sum
    shaders["computeShaderPrefix"]->use();
    glDispatchCompute(max_particles / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reorder
    shaders["computeShaderReorder"]->use();
    glDispatchCompute(max_particles / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // add trail particles
    /** what happens when we add more fireworks?
     * may need to add trails before prefix/reorder - make sure to flag trails as alive when adding
     * change dispatch to the number of alive particles (getAliveCount())
     * in computeShader, add another or extend aliveFlag SSBO to indicate whether trail or not
     * change particleTrail so it adds to aliveCount if the aliveparticle is a firework particle, else return early
     */
    /* second issue: since aliveCount contains trail particles, they are being updated as normal
     *      however, they are not differentiated from firework particles. May just include another
     *      SSBO for trail or not and another atomic counter for finding the right index to draw
     *      the trail particle.
     */
    /*shaders["computeShaderTrail"]->use();
    shaders["computeShaderTrail"]->setFloat("trailRate", 0.9f);
    shaders["computeShaderTrail"]->setUInt("maxParticle", max_particles);
    glDispatchCompute(particle_num / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

    /*glBindVertexArray(floorVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);	*/	
}

void Firework::resetAliveCount()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint initial = 0;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &initial);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void Firework::addFirework(const glm::vec3& origin)
{
    GLuint aliveIndex = getAliveCount();
    if (aliveIndex + particle_num < max_particles)
    {
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocity;
        for (unsigned int j = 0; j < particle_num; ++j)
        {
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(2.0f, 5.0f);
            velocity.emplace_back(speed * sin(phi) * cos(theta), speed * sin(phi) * sin(theta), speed * cos(phi), firework_lifetime);
            positions.emplace_back(origin, firework_lifetime);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * particle_num, positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * particle_num, velocity.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    else
        std::cout << "exceeded max particle num\n";
}

GLuint Firework::getAliveCount() const
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
    GLuint count = num[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    return count;
}

