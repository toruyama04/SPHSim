#include "Firework.h"

#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <vector>


float vertices[] = {
    // positions
     0.01f,  0.01f, 0.0f,
     0.01f, -0.01f, 0.0f,
    -0.01f, -0.01f, 0.0f,
    -0.01f,  0.01f, 0.0f
};

unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

Firework::Firework()
{
    firework_lifetime = 3.0f;
    _max_particles = 102400;

	initBuffers();
	initShaders();

    // make it able to take variable number of particles (think it can already)
    _neighbour_grids = std::make_unique<PointHashGridSearcher3>(5, 5, 5, 1000, maxNeighbourNum);
}

Firework::~Firework()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &positionsSSBO);
    glDeleteBuffers(1, &velocitiesSSBO);
    glDeleteBuffers(1, &aliveFlagSSBO);
    glDeleteBuffers(1, &atomicCounterBuffer);
    glDeleteBuffers(1, &drawIndirectBuffer);
    glDeleteBuffers(1, &forcesSSBO);
    glDeleteBuffers(1, &densitiesSSBO);
    glDeleteBuffers(1, &pressureSSBO);
    // glDeleteBuffers(1, &testSSBO);
}

void Firework::initShaders()
{
    particleShader = std::make_unique<Shader>("shaders/particle.vert", "shaders/particle.frag");
    densityUpdate = std::make_unique<Shader>("shaders/updateDensity.comp");
    viscosityUpdate = std::make_unique<Shader>("shaders/updateViscosity.comp");
    pressureCompute = std::make_unique<Shader>("shaders/computePressure.comp");
    pressureUpdate = std::make_unique<Shader>("shaders/updatePressure.comp");
    timeIntegrations = std::make_unique<Shader>("shaders/timeIntegration.comp");
    resetShader = std::make_unique<Shader>("shaders/resetShader.comp");
    velocityIntermediate = std::make_unique<Shader>("shaders/updateVelocity.comp");
}

void Firework::initBuffers()
{
    // initialising firework VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &positionsSSBO);
    glGenBuffers(1, &velocitiesSSBO);
    glGenBuffers(1, &aliveFlagSSBO);
    glGenBuffers(1, &forcesSSBO);
    glGenBuffers(1, &densitiesSSBO);
    glGenBuffers(1, &pressureSSBO);
	glGenBuffers(1, &drawIndirectBuffer);
    // glGenBuffers(1, &testSSBO);
    glGenBuffers(1, &atomicCounterBuffer);

	glBindVertexArray(VAO);

    // initialising vertices and corresponding indices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // if using textures
    /*glGenTextures(1, &particleTexture);
    glBindTexture(GL_TEXTURE_2D, particleTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("smoke.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        std::cout << "Failed to load texture\n";
    stbi_image_free(data);*/

    glBindVertexArray(0);

    // aliveCount
	// 0
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // initialising positions SSBO with default values: W component for current_lifetime
    // 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    std::vector default_values(_max_particles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_values.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);

    // initialising velocity SSBO with default values: W component for max_lifetime
    // 1
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    std::vector default_velocity(_max_particles, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_velocity.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, testSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_velocity.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, testSSBO);*/

    // 4
    std::vector<int> flag_default(_max_particles, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, aliveFlagSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * _max_particles, flag_default.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, aliveFlagSSBO);

    // 5
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forcesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_velocity.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, forcesSSBO);

    // 6
    std::vector<float> defaultFloat(_max_particles, 0.0f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _max_particles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, densitiesSSBO);

    // 7
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pressureSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _max_particles, defaultFloat.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, pressureSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 8
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    cmd = { 6, 1, 0, 0, 0};
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

void Firework::update(float delta_time)
{
    GLuint count = getAliveCount();
    GLuint groupNum = (count + 255) / 256;
    resetShader->use();
    resetShader->setUInt("particleNum", count);
    resetShader->setUInt("maxN", maxNeighbourNum);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // find all neighbours
    _neighbour_grids->build(count, this->_radius);

    // calculate non-pressure forces to find intermediate velocity v*
    // use Eq 8
    viscosityUpdate->use();
    float cubicSpline = 1 / (2 * 3.14159265359f * std::pow(_radius, 3.0f));
	viscosityUpdate->setFloat("cubicSpline", cubicSpline);
	viscosityUpdate->setFloat("viscosityCoefficient", 1.0f);
	viscosityUpdate->setFloat("h", _radius);
	viscosityUpdate->setUInt("particleNum", count);
	viscosityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
	viscosityUpdate->setFloat("mass", _mass);
	glDispatchCompute(groupNum, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    velocityIntermediate->use();
    velocityIntermediate->setFloat("dt", delta_time);
    velocityIntermediate->setFloat("mass", _mass);
    velocityIntermediate->setUInt("particleNum", count);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // calculate new density using intermediate velocity v*
    // use Eq in Algo 2
    densityUpdate->use();
    densityUpdate->setFloat("h", this->_radius);
    densityUpdate->setFloat("mass", _mass);
    densityUpdate->setUInt("particleNum", count);
    densityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    densityUpdate->setFloat("cubicSpline", cubicSpline);
    densityUpdate->setFloat("dt", delta_time);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // use Eq 9
    pressureCompute->use();
    pressureCompute->setFloat("targetDensity", _targetDensity);
    pressureCompute->setFloat("k", 10000.0f);
    pressureCompute->setFloat("negativePressureScale", 0.0);
    pressureCompute->setUInt("particleNum", count);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // use Eq 6
    pressureUpdate->use();
    pressureUpdate->setFloat("cubicSpline", cubicSpline);
    pressureUpdate->setFloat("h", _radius);
    pressureUpdate->setUInt("particleNum", count);
    pressureUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    pressureUpdate->setFloat("mass", _mass);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    timeIntegrations->use();
    timeIntegrations->setFloat("mass", _mass);
    timeIntegrations->setFloat("dt", delta_time);
    timeIntegrations->setUInt("particleNum", count);
    timeIntegrations->setVec3("boundaryMin", glm::vec3(0.0, 0.0, 0.0));
    timeIntegrations->setVec3("boundaryMax", glm::vec3(10.0, 10.0, 10.0));
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

void Firework::render(const glm::mat4& view, const glm::mat4& projection)
{
    GLuint count = getAliveCount();
    count += 10;
    cmd = { 6, count, 0, 0, 0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(cmd), &cmd);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    particleShader->use();
    glm::mat4 model = glm::mat4(1.0f);
    particleShader->setMat4("model", model);
    particleShader->setMat4("view", view);
    particleShader->setMat4("projection", projection);

    if (count > 0)
    {
        glBindVertexArray(VAO);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
    glBindVertexArray(0);

    /*
     * to render 3D fluids, the surface was extracted by taking the iso-surface from the SPH density field.
     * If fluid density is p, p/2 is taken for the isoSurface.
     * then converted to a triangle mesh using marching cubes algorithm
     * rendered using a path-tracing renderer
     */


     /*shaders["floorShader"]->use();
     model = glm::mat4(1.0f);
     model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
     model = glm::scale(model, glm::vec3(2.0f));
     shaders["floorShader"]->setMat4("model", model);
     shaders["floorShader"]->setMat4("view", view);
     shaders["floorShader"]->setMat4("projection", projection);*/
}

void Firework::resetAliveCount(GLuint amount)
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint count = num[0];
    count -= amount;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &count);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

/*void Firework::addFirework(const glm::vec3& origin)
{
    GLuint aliveIndex = getAliveCount();
    if (aliveIndex + _particle_num < _max_particles)
    {
        ++_firework_num;
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocity;
        for (unsigned int j = 0; j < _particle_num; ++j)
        {
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(3.0f, 5.0f);
            velocity.emplace_back(speed * sin(phi) * cos(theta), speed * sin(phi) * sin(theta), speed * cos(phi), firework_lifetime);
            positions.emplace_back(origin, firework_lifetime);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * _particle_num, positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * _particle_num, velocity.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
        GLuint partn = _particle_num;
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &partn);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }
    else
        std::cout << "exceeded max particle num\n";
}*/

float randomFloat(float min, float max) {
    static std::default_random_engine e;
    static std::uniform_real_distribution<float> dis(min, max); // range [min, max]
    return dis(e);
}

void Firework::addParticleCube(const glm::vec3& origin, float spacing, int particlesPerSide)
{
    GLuint aliveIndex = getAliveCount();
    int totalParticles = particlesPerSide * particlesPerSide * particlesPerSide;

    if (aliveIndex + totalParticles < _max_particles)
    {
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocities;

        float halfSide = (particlesPerSide - 1) * spacing / 2.0f;
        float gap = 0.1f; // Define the gap between the two halves
        float baseVelocityMagnitude = 0.05f; // Base uniform velocity magnitude (towards the other half)
        float velocityVariation = 0.02f; // Variation in velocity magnitude

        // Loop to create particles in the split cube
        for (int x = 0; x < particlesPerSide; ++x)
        {
            for (int y = 0; y < particlesPerSide; ++y)
            {
                for (int z = 0; z < particlesPerSide; ++z)
                {
                    // Calculate particle position
                    glm::vec3 pos = origin + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                    // Adjust the particle position based on whether it's in the left or right half
                    if (pos.x < origin.x) {
                        pos.x -= gap / 2.0f;  // Shift left half particles further left by half the gap
                    }
                    else {
                        pos.x += gap / 2.0f;  // Shift right half particles further right by half the gap
                    }

                    positions.emplace_back(pos, firework_lifetime);

                    // Set velocities pointing towards the opposite half with a slight random variation
                    glm::vec3 velocity;
                    float randomVelocity = baseVelocityMagnitude + randomFloat(-velocityVariation, velocityVariation);
                    if (pos.x < origin.x) {
                        velocity = glm::vec3(randomVelocity, randomFloat(-velocityVariation, velocityVariation), randomFloat(-velocityVariation, velocityVariation)); // Left half particles move right
                    }
                    else {
                        velocity = glm::vec3(-randomVelocity, randomFloat(-velocityVariation, velocityVariation), randomFloat(-velocityVariation, velocityVariation)); // Right half particles move left
                    }

                    velocities.emplace_back(velocity, firework_lifetime);
                }
            }
        }

        positions.emplace_back(0.0, 5.0, 5.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(0.0, 0.0, 0.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(10.0, 0.0, 0.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(0.0, 0.0, 10.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(10.0, 0.0, 10.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(10.0, 10.0, 10.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(10.0, 10.0, 0.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(0.0, 10.0, 10.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(0.0, 10.0, 0.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        positions.emplace_back(5.0, 5.0, 0.0, firework_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, firework_lifetime);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (totalParticles + 10), positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (totalParticles + 10), velocities.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
        GLuint newAliveCount = aliveIndex + totalParticles;
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &newAliveCount);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        // set aliveFlags
    }
    else
    {
        std::cout << "Exceeded max particle num\n";
    }
}

GLuint Firework::getAliveCount()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint count = num[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    return count;
}

