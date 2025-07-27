#include "SPHSim.h"

#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <vector>
#include <unordered_set>


float vertices[] = {
    // positions
     0.003f,  0.003f, 0.0f,
     0.003f, -0.003f, 0.0f,
    -0.003f, -0.003f, 0.0f,
    -0.003f,  0.003f, 0.0f
};

unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

Sim::Sim(glm::vec3 origin, glm::vec3 gridExtent)
{
    this->origin = origin;
    this->extents = gridExtent;

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
    glDeleteBuffers(1, &drawIndirectBuffer);
    glDeleteBuffers(1, &forcesSSBO);
    glDeleteBuffers(1, &densitiesSSBO);
    delete particleShader;
    delete densityUpdate;
    delete viscosityUpdate;
    delete pressureUpdate;
    delete timeIntegrations;
    delete neighbourGrid;
    delete positionPredict;
}

void Sim::initShaders()
{
    particleShader = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/particle.vert", "C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/particle.frag", nullptr);
    densityUpdate = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updateDensity.comp");
    viscosityUpdate = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updateViscosity.comp");
    pressureUpdate = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updatePressure.comp");
    timeIntegrations = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/timeIntegration.comp");
    positionPredict = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/updatePredictedPos.comp");
}

void Sim::initBuffers()
{
    // initialising VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
	glGenBuffers(1, &drawIndirectBuffer);

    // initialising vertices and corresponding indices
	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);

    // 0: for drawing primitives quickly
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

void Sim::initSSBO()
{
    glGenBuffers(1, &positionsSSBO);
    glGenBuffers(1, &velocitiesSSBO);
    glGenBuffers(1, &forcesSSBO);
    glGenBuffers(1, &densitiesSSBO);
    glGenBuffers(1, &predPositionsSSBO);

    // 0: initialising positions SSBO:
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * (totalParticles + 8), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionsSSBO);

    // 1: initialising velocity SSBO:
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocitiesSSBO);

    // 2: initialising predicted position SSBO:
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predPositionsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predPositionsSSBO);

    /*
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictedVelocitySSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * fluidParticleNum, zeroed_outF.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predictedVelocitySSBO);*/

    // 3: initialising forcesSSBO buffer: used for computing resulting velocity
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forcesSSBO);
    std::vector<glm::vec4> zeroed_outF(fluidParticleNum, glm::vec4(0.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * fluidParticleNum, zeroed_outF.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, forcesSSBO);

    // 4: initialising densitiesSSBO buffer: holds densities for each fluid particle
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, densitiesSSBO);


    GLuint count = totalParticles + 8;
    cmd = { 6, count, 0, 0, 0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
    glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(cmd), &cmd);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

GLuint Sim::addBoundaryParticles(std::vector<glm::vec4>& positions, float spacing, int layers)
{
    glm::vec3 size = extents - origin;

    // split cube into cells with spacing
    int n = static_cast<int>(size.x / spacing);

    GLuint count = 0;
    
    // we use unordered set to make sure we don't have overlapping boundary particles
    std::unordered_set<uint64_t> placed;
    
    // our boundary particles aren't actually on the exact boundary
    glm::vec3 start = origin + glm::vec3(0.5f * spacing);
    glm::vec3 end = extents - glm::vec3(0.5f * spacing);

    auto hash = [](int x, int y, int z) -> uint64_t {
        return (static_cast<uint64_t>(x) << 40) | (static_cast<uint64_t>(y) << 20) | static_cast<uint64_t>(z);
        };

    // go through all grid cells
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {

                // skip grid cells which aren't within layers from the boundary
                bool isBoundary =
                    (i < layers) || (i >= n - layers) ||
                    (j < layers) || (j >= n - layers) ||
                    (k < layers) || (k >= n - layers);
                if (!isBoundary) continue;

                uint64_t h = hash(i, j, k);
                if (placed.find(h) != placed.end()) continue;

                placed.insert(h);
                glm::vec3 pos = start + glm::vec3(i, j, k) * spacing;
                if (pos.x > end.x || pos.y > end.y || pos.z > end.z) continue;

                positions.push_back(glm::vec4(pos, 0.35f));
                ++count;
            }
        }
    }
    return count;
}


void Sim::update(float delta_time)
{
    // buffer contents
    // [boundary particles : fluid particles : grid visual particles]
    // neighbour buffers + positions + velocity : total particles
    // predictedVelocity, forces, densities : fluid particles
    GLuint groupNum = (fluidParticleNum + 255) / 256;
    GLuint groupNumWithBoundary = (totalParticles + 255) / 256;

    // find all neighbours and build neighbourList
    neighbourGrid->build(this->radius);

    positionPredict->use();
    positionPredict->setUInt("totalParticleNum", totalParticles);
    positionPredict->setUInt("boundaryParticleNum", boundaryParticleNum);
    positionPredict->setFloat("dt", delta_time);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    // reconstruct density
    float h3 = radius * radius * radius;
    float kernNorm = 8.0f / (3.14159265359f * h3);
    densityUpdate->use();
    densityUpdate->setFloat("h", this->radius);
    densityUpdate->setFloat("h3", h3);
    densityUpdate->setFloat("mass", particleMass);
    densityUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    densityUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    densityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    densityUpdate->setFloat("sigma", kernNorm);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);


    float kinematicViscosity = 0.0001f / 1000.0f;
    float kernel_grad = 48 / (3.14159265359f * h3 * radius);
    viscosityUpdate->use();
    viscosityUpdate->setFloat("kviscosity", kinematicViscosity);
    viscosityUpdate->setFloat("h", radius);
    viscosityUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    viscosityUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    viscosityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    viscosityUpdate->setFloat("mass", particleMass);
    viscosityUpdate->setFloat("kernelg", kernel_grad);
    viscosityUpdate->setFloat("dt", delta_time);
    viscosityUpdate->setVec3("gravity", glm::vec3(0.0, -0.5, 0.0));
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    pressureUpdate->use();
    pressureUpdate->setFloat("restDensity", targetDensity);
    pressureUpdate->setFloat("h", radius);
    pressureUpdate->setFloat("k1", 100.0f);
    pressureUpdate->setFloat("k2", 7.0f);
    pressureUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    pressureUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    pressureUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    pressureUpdate->setFloat("kernelg", kernel_grad);
    pressureUpdate->setFloat("mass", particleMass);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);


    timeIntegrations->use();
    timeIntegrations->setFloat("dt", delta_time);
    timeIntegrations->setUInt("totalParticles", totalParticles);
    timeIntegrations->setUInt("boundaryParticleNum", boundaryParticleNum);
    timeIntegrations->setUInt("fluidParticleNum", fluidParticleNum);
    timeIntegrations->setFloat("mass", particleMass);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

void Sim::render(const glm::mat4& view, const glm::mat4& projection)
{
    particleShader->use();
    glm::mat4 model = glm::mat4(1.0f);
    particleShader->setMat4("model", model);
    particleShader->setMat4("view", view);
    particleShader->setMat4("projection", projection);
    
    glBindVertexArray(VAO);
    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindVertexArray(0);

     /*shaders["floorShader"]->use();
     model = glm::mat4(1.0f);
     model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
     model = glm::scale(model, glm::vec3(2.0f));
     shaders["floorShader"]->setMat4("model", model);
     shaders["floorShader"]->setMat4("view", view);
     shaders["floorShader"]->setMat4("projection", projection);*/
}

/*void Sim::addSim(const glm::vec3& origin, const GLuint particle_num)
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
}*/

void Sim::addParticleCube(const glm::vec3 center, float spacing, GLuint particlesPerSide)
{
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> velocities;

    fluidParticleNum = particlesPerSide * particlesPerSide * particlesPerSide;
    boundaryParticleNum = addBoundaryParticles(positions, 0.1, 2);
    for (int i = 0; i < boundaryParticleNum; i++)
    {
        velocities.push_back(glm::vec4(0.0f));
    }

    totalParticles = fluidParticleNum + boundaryParticleNum;

    std::cout << "boundary: " << boundaryParticleNum << " fluid: " << fluidParticleNum << "\n";
    int index = 0;
    float halfSide = (particlesPerSide - 1) * spacing * 0.5f;

    for (int x = 0; x < particlesPerSide && index < fluidParticleNum; ++x) {
        for (int y = 0; y < particlesPerSide && index < fluidParticleNum; ++y) {
            for (int z = 0; z < particlesPerSide && index < fluidParticleNum; ++z) {
                glm::vec3 pos = center + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                positions.push_back(glm::vec4(pos, 1.0f));
                velocities.push_back(glm::vec4(0.0f));

                ++index;
            }
        }
    }
    // edges of the grid for visualisation (static) 
    {
        positions.push_back(glm::vec4(origin, 0.5f));
        positions.push_back(glm::vec4(extents.x, origin.y, origin.z, 0.1f));
        positions.push_back(glm::vec4(extents.x, extents.y, origin.z, 0.1f));
        positions.push_back(glm::vec4(extents.x, origin.y, extents.z, 0.1f));
        positions.push_back(glm::vec4(origin.x, extents.y, extents.z, 0.1f));
        positions.push_back(glm::vec4(origin.x, origin.y, extents.z, 0.1f));
        positions.push_back(glm::vec4(origin.x, extents.y, origin.z, 0.1f));
        positions.push_back(glm::vec4(extents, 0.5f));
    }

    initSSBO();
    neighbourGrid = new Grid(origin, extents, totalParticles, fluidParticleNum, maxNeighbourNum, 0.25);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * (totalParticles + 8), positions.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * (totalParticles), velocities.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predPositionsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * boundaryParticleNum, positions.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    std::vector<float> defaultDensities(totalParticles, targetDensity);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * totalParticles, defaultDensities.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // initialise mass, radius, target density
    //particleMass = targetDensity * std::pow(spacing, 3.0f);
    particleMass = 8.0f;
}


