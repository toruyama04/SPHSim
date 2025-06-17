#include "SPHSim.h"

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

Sim::Sim()
{
    sim_lifetime = 6.0f;
    _max_particles = 10000;
    // _mass = _targetDensity * std::pow(_radius, 3.0f);

	initBuffers();
	initShaders();
}

Sim::~Sim()
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

void Sim::initShaders()
{
    particleShader = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/particle.vert", "C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/particle.frag");
    densityUpdate = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updateDensity.comp");
    viscosityUpdate = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updateViscosity.comp");
    pressureCompute = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/computePressure.comp");
    pressureUpdate = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updatePressure.comp");
    timeIntegrations = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/timeIntegration.comp");
    resetShader = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/resetShader.comp");
    velocityIntermediate = std::make_unique<Shader>("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updateVelocity.comp");
}

void Sim::initBuffers()
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
    glGenBuffers(1, &testSSBO);
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

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, testSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * _max_particles, default_velocity.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, testSSBO);

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
    std::vector<float> defaultDensity(_max_particles, 1000.0f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * _max_particles, defaultDensity.data(), GL_DYNAMIC_DRAW);
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

void Sim::update(float delta_time)
{
    GLuint count = getAliveCount();
    GLuint groupNum = (count + 255) / 256;
    resetShader->use();
    resetShader->setUInt("particleNum", count);
    resetShader->setUInt("maxN", maxNeighbourNum);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // find all neighbours
    _neighbour_grids->build(count, this->_radius);

    // calculate non-pressure forces to find intermediate velocity v*
    // use Eq 8
    float poly6lap = 45.0f / (3.14159265359f * std::pow(_radius, 6.0f));

    /*velocityIntermediate->use();
    velocityIntermediate->setFloat("dt", delta_time);
    velocityIntermediate->setFloat("mass", _mass);
    velocityIntermediate->setUInt("particleNum", count);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);*/

    // calculate new density using intermediate velocity v*
    // use Eq in Algo 2
    float poly6 = 315.0f / (64.0f * 3.14159265359f * std::pow(_radius, 9.0f));
    float poly6mass = poly6 * _mass;
    densityUpdate->use();
    densityUpdate->setFloat("h", this->_radius);
    densityUpdate->setFloat("h2", _radius * _radius);
    densityUpdate->setFloat("mass", _mass);
    densityUpdate->setUInt("particleNum", count);
    densityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    densityUpdate->setFloat("poly6mass", poly6mass);
    densityUpdate->setFloat("p0", _targetDensity);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // use Eq 9
    pressureCompute->use();
    pressureCompute->setFloat("targetDensity", _targetDensity);
    pressureCompute->setFloat("k", 100.0f);
    pressureCompute->setUInt("particleNum", count);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // use Eq 6
    float spikyGrad = -45.0f / (3.14159265359f * std::pow(_radius, 6.0f));
    /*pressureUpdate->use();
    pressureUpdate->setFloat("h", _radius);
    pressureUpdate->setUInt("particleNum", count);
    pressureUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    pressureUpdate->setFloat("spikyGrad", spikyGrad);
    pressureUpdate->setFloat("mass", _mass);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);*/

    /*viscosityUpdate->use();
    viscosityUpdate->setFloat("viscosityCoefficient", poly6lap);
    viscosityUpdate->setFloat("h", _radius);
    viscosityUpdate->setUInt("particleNum", count);
    viscosityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    viscosityUpdate->setFloat("mass", _mass);
    viscosityUpdate->setFloat("e", 0.3f);
    viscosityUpdate->setVec3("grav", glm::vec3(0.0f, -9.8f, 0.0f));
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);*/

    timeIntegrations->use();
    timeIntegrations->setFloat("dt", delta_time);
    timeIntegrations->setUInt("particleNum", count);
    timeIntegrations->setVec3("boundaryMin", glm::vec3(0.0, 0.0, 0.0));
    timeIntegrations->setVec3("boundaryMax", glm::vec3(10.0, 10.0, 10.0));
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

void Sim::render(const glm::mat4& view, const glm::mat4& projection)
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

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glm::vec4* velocityData = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4), GL_MAP_READ_BIT);
    std::cout << velocityData[0].w <<  "\n";
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/

    if (count > 0)
    {
        glBindVertexArray(VAO);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
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

void Sim::resetAliveCount(GLuint amount)
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint count = num[0];
    count -= amount;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &count);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void Sim::addSim(const glm::vec3& origin, const GLuint particle_num)
{
    GLuint aliveIndex = getAliveCount();
    _neighbour_grids = std::make_unique<PointHashGridSearcher3>(10, 10, 10, particle_num, maxNeighbourNum);
    if (aliveIndex + particle_num < _max_particles)
    {
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocities;

        float initialRadius = 0.5f;

        for (unsigned int j = 0; j < particle_num; ++j)
        {
            glm::vec3 randomDir = glm::sphericalRand(1.0f);  // Unit sphere
            float randomDist = glm::linearRand(0.0f, initialRadius);  // Random distance within radius
            glm::vec3 randomPos = origin + randomDir * randomDist;

            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = 0.3f;  // Adjust speed range if needed
            glm::vec3 velocity = speed * glm::vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));

            velocities.emplace_back(velocity, sim_lifetime);
            positions.emplace_back(randomPos, sim_lifetime);
        }
        positions.emplace_back(0.0, 5.0, 5.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 0.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 0.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 0.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 0.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 10.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 10.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 10.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 10.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(5.0, 5.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (particle_num + 10), positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (particle_num + 10), velocities.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
        GLuint newAliveCount = aliveIndex + particle_num;
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &newAliveCount);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        _mass = _targetDensity * std::pow(0.1f, 3.0f);
    }
    else
        std::cout << "exceeded max particle num\n";
}

void Sim::addParticleCube(const glm::vec3& origin, float spacing, int particlesPerSide)
{
    GLuint aliveIndex = getAliveCount();
    int particleCount = particlesPerSide * particlesPerSide * particlesPerSide;
    _neighbour_grids = std::make_unique<PointHashGridSearcher3>(10, 10, 10, particleCount, maxNeighbourNum);

    if (aliveIndex + particleCount < _max_particles)
    {
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> velocities;

        int side = particlesPerSide;
        int index = 0;
        float halfSide = (side - 1) * spacing * 0.5f;
        float offsetMagnitude = spacing * 0.2f; // Random offset scale, 20% of spacing

        for (int x = 0; x < side; ++x) {
            for (int y = 0; y < side; ++y) {
                for (int z = 0; z < side; ++z) {
                    if (index >= particleCount) break;

                    // Base particle position calculated with the origin as the center
                    glm::vec3 pos = origin + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                    // Generate small random offset in each direction, scaled by offsetMagnitude
                    glm::vec3 randomOffset = glm::vec3(
                        (rand() / (float)RAND_MAX - 0.5f) * offsetMagnitude,
                        (rand() / (float)RAND_MAX - 0.5f) * offsetMagnitude,
                        (rand() / (float)RAND_MAX - 0.5f) * offsetMagnitude
                    );

                    // Add the random offset to the particle position
                    pos += randomOffset;

                    // Direction is calculated from the particle to the origin (center)
                    glm::vec3 dir = glm::vec3(0.0);  // For now, keeping direction zero

                    // Store the position and velocity in the buffers
                    positions.push_back(glm::vec4(pos, sim_lifetime));
                    velocities.push_back(glm::vec4(dir, sim_lifetime));

                    ++index;
                }
            }
        }

        positions.emplace_back(0.0, 5.0, 5.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 0.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 0.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 0.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 0.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 10.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(10.0, 10.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 10.0, 10.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(0.0, 10.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        positions.emplace_back(5.0, 5.0, 0.0, sim_lifetime);
        velocities.emplace_back(0.0, 0.0, 0.0, sim_lifetime);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (particleCount + 10), positions.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, aliveIndex * sizeof(glm::vec4), sizeof(glm::vec4) * (particleCount + 10), velocities.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
        GLuint newAliveCount = aliveIndex + particleCount;
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &newAliveCount);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        // set aliveFlags
        _mass = _targetDensity * std::pow(spacing, 3.0f);
    }
    else
    {
        std::cout << "Exceeded max particle num\n";
    }
}

GLuint Sim::getAliveCount()
{
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint count = num[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    return count;
}

