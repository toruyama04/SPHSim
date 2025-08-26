#include "SPHSim.h"

#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <vector>
#include <unordered_set>

// square
float vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};

Sim::Sim(glm::vec3 origin, glm::vec3 gridExtent)
    : origin(origin),
      extents(gridExtent)
{
	initBuffers();
	initShaders();
}

Sim::~Sim() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &positionsSSBO);
    glDeleteBuffers(1, &velocitiesSSBO);
    glDeleteBuffers(1, &forcesSSBO);
    glDeleteBuffers(1, &densitiesSSBO);
    glDeleteBuffers(1, &predPositionsSSBO);
    glDeleteBuffers(1, &colourSSBO);
    // may change to smart pointers
    delete particleShader;
    delete densityUpdate;
    delete viscosityUpdate;
    delete pressureUpdate;
    delete timeIntegrations;
    delete neighbourGrid;
    delete positionPredict;
}

void Sim::initShaders() {
    // using shader class from learnopengl.com
    particleShader = new Shader("shaders/particle.vert", "shaders/particle.frag", nullptr);
    densityUpdate = new Shader("shaders/updateDensity.comp");
    viscosityUpdate = new Shader("shaders/updateViscosity.comp");
    pressureUpdate = new Shader("shaders/updatePressure.comp");
    timeIntegrations = new Shader("shaders/timeIntegration.comp");
    positionPredict = new Shader("shaders/updatePredictedPos.comp");
}

void Sim::initBuffers() {
    // vertex array object and vertices stuff
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Sim::initSSBO() {
    /*  1. first generate the buffer
        2. bind the specific SSBO to the current GL_SHADER_STORAGE_BUFFER
        3. specify the data layout, size, and the default data if necessary
        4. assign a unique buffer identifier number (BufferBase) */
    
    glGenBuffers(1, &positionsSSBO);
    glGenBuffers(1, &velocitiesSSBO);
    glGenBuffers(1, &forcesSSBO);
    glGenBuffers(1, &densitiesSSBO);
    glGenBuffers(1, &predPositionsSSBO);
    glGenBuffers(1, &colourSSBO);

    // important to note which buffers have data for just fluid/boundary particles or both

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

    // 3: initialising forcesSSBO buffer: used for computing resulting velocity
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, forcesSSBO);
    std::vector<glm::vec4> zeroed_outF(fluidParticleNum, glm::vec4(0.0f));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * fluidParticleNum, zeroed_outF.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, forcesSSBO);

    // 4: initialising densitiesSSBO buffer:
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, densitiesSSBO);

    // 14: initialising colour buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * (totalParticles + 8), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, colourSSBO);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

GLuint Sim::addBoundaryParticles(std::vector<glm::vec4>& positions, float spacing, int layers) {
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
                bool isBoundary = (i < layers) || (i >= n - layers) || (j < layers) || (j >= n - layers) ||
                    (k < layers) || (k >= n - layers);
                if (!isBoundary) continue;

                uint64_t h = hash(i, j, k);
                if (placed.find(h) != placed.end()) continue;

                placed.insert(h);
                glm::vec3 pos = start + glm::vec3(i, j, k) * spacing;
                if (pos.x > end.x || pos.y > end.y || pos.z > end.z) continue;

                positions.push_back(glm::vec4(pos, 0.0f));
                ++count;
            }
        }
    }
    return count;
}

void Sim::addParticleCube(const glm::vec3 center, float spacing, GLuint particlesPerSide)  {
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> velocities;
    std::vector<glm::vec4> colours;

    // mass 
    particleMass = targetDensity * std::pow(spacing, 3.0f);
    fluidParticleNum = particlesPerSide * particlesPerSide * particlesPerSide;

    // spacing and layers - particle-based boundary conditions
    boundaryParticleNum = addBoundaryParticles(positions, 0.072, 1);
    totalParticles = fluidParticleNum + boundaryParticleNum;

    for (int i = 0; i < boundaryParticleNum; i++)
    {
        // fill velocities to reduce calculations in shaders, remove for less memory
        velocities.push_back(glm::vec4(0.0f));
        // faint outline
        colours.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));
    }

    // we position particles from a center point given as parameter
    int index = 0;
    float halfSide = (particlesPerSide - 1) * spacing * 0.5f;

    for (int x = 0; x < particlesPerSide; ++x) {
        for (int y = 0; y < particlesPerSide; ++y) {
            for (int z = 0; z < particlesPerSide; ++z) {
                if (index >= fluidParticleNum) break;
                glm::vec3 pos = center + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                positions.push_back(glm::vec4(pos, 1.0f));
                velocities.push_back(glm::vec4(0.0f));
                colours.push_back(glm::vec4(0.2f, 0.2f, 1.0f, 0.8f));

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
        for (int i = 0; i < 8; ++i)
            colours.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 0.3f));
    }

    // initialise gpu buffers to store particle specific data, assign data afterwards
    initSSBO();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * (totalParticles + 8), positions.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitiesSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * (totalParticles), velocities.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predPositionsSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * boundaryParticleNum, positions.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * (totalParticles + 8), colours.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densitiesSSBO);
    std::vector<float> defaultDensities(totalParticles, targetDensity);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * totalParticles, defaultDensities.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // last but not least, creating the neighbour grid 
    neighbourGrid = new Grid(origin, extents, totalParticles, fluidParticleNum, maxNeighbourNum, 0.25);
}


void Sim::update(float delta_time) {
    // buffer contents order - if buffer doesn't contain one or more data, remove from list
    // [boundary particles , fluid particles , grid visual particles]

    // dispatch count based on fluid or fluid+boundary
    GLuint groupNum = (fluidParticleNum + 255) / 256;
    GLuint groupNumWithBoundary = (totalParticles + 255) / 256;

    // find all neighbours and build neighbourList
    neighbourGrid->build(this->radius);

    // an improvement where we compute fluid data based on particle's trajectory
    positionPredict->use();
    positionPredict->setUInt("totalParticleNum", totalParticles);
    positionPredict->setUInt("boundaryParticleNum", boundaryParticleNum);
    positionPredict->setFloat("dt", delta_time);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    /* reconstruct density
    *   - establish a poly6 kernel for density estimation
    *   - pre-calculate sigma value to reduce computation in shaders
    */
    float h2 = radius * radius;
    float kernNorm = 315.0f / (64.0 * 3.14159265359f * std::pow(radius, 9.0));
    densityUpdate->use();
    densityUpdate->setFloat("h", this->radius);
    densityUpdate->setFloat("h2", h2);
    densityUpdate->setFloat("mass", particleMass);
    densityUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    densityUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    densityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    densityUpdate->setFloat("sigma", kernNorm);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    /* compute predicted velocity, incorporates external forces
    *   - laplacian kernel for viscosity 
    */
    viscosityUpdate->use();
    viscosityUpdate->setFloat("kviscosity", kviscosity);
    viscosityUpdate->setFloat("h", radius);
    viscosityUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    viscosityUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    viscosityUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    viscosityUpdate->setFloat("mass", particleMass);
    viscosityUpdate->setFloat("sigma", 45.0f / (3.14159265359f * std::pow(radius,6.0)));
    viscosityUpdate->setFloat("dt", delta_time);
    viscosityUpdate->setVec3("gravity", glm::vec3(0.0, -9.8, 0.0));
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    /* compute pressure force
    *   - spiky kernel
    */
    pressureUpdate->use();
    pressureUpdate->setFloat("restDensity", targetDensity);
    pressureUpdate->setFloat("h", radius);
    pressureUpdate->setFloat("k1", targetDensity);
    pressureUpdate->setFloat("k2", 7.0f);
    pressureUpdate->setUInt("boundaryParticleNum", boundaryParticleNum);
    pressureUpdate->setUInt("fluidParticleNum", fluidParticleNum);
    pressureUpdate->setUInt("maxNeighbourNum", maxNeighbourNum);
    pressureUpdate->setFloat("sigma", -45.0 / (3.14159265359 * std::pow(radius, 6.0)));
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

void Sim::render(const glm::mat4& view, const glm::mat4& projection) {
    particleShader->use();
    glm::mat4 model = glm::mat4(1.0f);
    particleShader->setMat4("model", model);
    particleShader->setMat4("view", view);
    particleShader->setMat4("projection", projection);
    particleShader->setFloat("particleSphereSize", sphereRadius);
    
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, totalParticles + 8);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindVertexArray(0);
}



