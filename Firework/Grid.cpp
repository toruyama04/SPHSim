#include "Grid.h"
#include <vector>

Grid::Grid(glm::vec3 gridOrigin, glm::vec3 extents, GLuint totalParticles, GLuint fluidParticles,
    GLuint maxNeighbourNum, float gridSpacing)
{
    glGenBuffers(1, &particleNumPerBinSSBO);
    glGenBuffers(1, &binIndexForParticleSSBO);
    glGenBuffers(1, &prefixForBinReorderSSBO);
    glGenBuffers(1, &particlesOrderedByBinSSBO);
    glGenBuffers(1, &flatNeighboursSSBO);
    glGenBuffers(1, &prefixIndexCounterSSBO);
    glGenBuffers(1, &neighbourListSSBO);
    glGenBuffers(1, &endIndexNeighbourSSBO);
    glGenBuffers(1, &neighbourBinOffsetSSBO);

    GLuint resolution = GLuint((extents.x - gridOrigin.x) / gridSpacing);

    binCount = resolution * resolution * resolution;
    resolutionVec = glm::vec3(resolution, resolution, resolution);
    this->maxNeighbourNum = maxNeighbourNum;
    this->gridSpacing = gridSpacing;
    this->gridOrigin = gridOrigin;
    this->totalParticles = totalParticles;
    this->fluidParticleNum = fluidParticles;
    this->boundaryParticleNum = totalParticles - fluidParticles;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNumPerBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * binCount, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleNumPerBinSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binIndexForParticleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, binIndexForParticleSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixForBinReorderSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * (binCount + 1), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, prefixForBinReorderSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesOrderedByBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * totalParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, particlesOrderedByBinSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixIndexCounterSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * binCount, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, prefixIndexCounterSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourListSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * fluidParticles * maxNeighbourNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, neighbourListSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, endIndexNeighbourSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * fluidParticles, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, endIndexNeighbourSSBO);

    // computing the neighbouring bin indices for all bins for flatNeighbours - only need to compute once
    std::vector<GLuint> neighbourOffsets(binCount + 1, 0);
    std::vector<GLuint> flatNeighbours;

    flatNeighbours.reserve(binCount * 27);

    GLuint offset = 0;

    for (unsigned int z = 0; z < resolution; ++z) {
        for (unsigned int y = 0; y < resolution; ++y) {
            for (unsigned int x = 0; x < resolution; ++x) {

                GLuint binIndex = x + y * resolution + z * resolution * resolution;
                neighbourOffsets[binIndex] = offset;

                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            int nx = int(x) + dx;
                            int ny = int(y) + dy;
                            int nz = int(z) + dz;

                            if (nx >= 0 && nx < int(resolution) &&
                                ny >= 0 && ny < int(resolution) &&
                                nz >= 0 && nz < int(resolution)) {

                                GLuint neighbourIdx = nx + ny * resolution + nz * resolution * resolution;
                                flatNeighbours.push_back(neighbourIdx);
                                ++offset;
                            }
                        }
                    }
                }
            }
        }
    }

    neighbourOffsets[binCount] = offset;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flatNeighboursSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * flatNeighbours.size(), flatNeighbours.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, flatNeighboursSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourBinOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * neighbourOffsets.size(), neighbourOffsets.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, neighbourBinOffsetSSBO);

    countShader = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/count.comp");
    reorderShader = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/reorder.comp");
    buildNeighbourListShader = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/buildNeighbourList.comp");
    computePrefixSumShader = new Shader("C:/Users/toruy_iu/source/repos/Firework/Firework/shaders/computePrefixSum.comp");
}

Grid::~Grid()
{
    delete countShader;
    delete reorderShader;
    delete buildNeighbourListShader;
    delete computePrefixSumShader;
}

void Grid::build(float searchRadius) {
    // we dispatch 256 threads in x for a compute shader over all particles
    GLuint groupNumAll = (totalParticles + 255) / 256;
    GLuint groupNumFluid = (fluidParticleNum + 255) / 256;

    glClearNamedBufferData(particleNumPerBinSSBO, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glClearNamedBufferData(prefixIndexCounterSSBO, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);


    // setting bin index for each particle, counting particles per bin
    countShader->use();
    countShader->setVec3("gridResolution", resolutionVec);
    countShader->setVec3("gridOrigin", gridOrigin);
    countShader->setFloat("gridSpacing", gridSpacing);
    countShader->setUInt("totalParticles", totalParticles);
    glDispatchCompute(groupNumAll, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Computing prefix indices on GPU - also retting prefixCounter
    computePrefixSumShader->use();
    computePrefixSumShader->setUInt("binCount", binCount);
    computePrefixSumShader->setUInt("totalParticles", totalParticles);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // creating a particlesOrderedByBin buffer using prefix sum 
    reorderShader->use();
    reorderShader->setUInt("totalParticles", totalParticles);
    glDispatchCompute(groupNumAll, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /** Build the neighbour list
    *    the idea is to only search for nearby particles in the surrounding grid bins
    *    instead of checking between every particle
    */ 
    buildNeighbourListShader->use();
    buildNeighbourListShader->setFloat("searchRadius", searchRadius);
    buildNeighbourListShader->setUInt("maxNeighboursPerParticle", maxNeighbourNum);
    buildNeighbourListShader->setUInt("boundaryParticleNum", boundaryParticleNum);
    buildNeighbourListShader->setUInt("totalParticles", totalParticles);
    glDispatchCompute(groupNumFluid, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
