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

    GLuint resolution = GLuint((extents.x - gridOrigin.x) / gridSpacing);
    binCount = resolution * resolution * resolution;
    resolutionVec = glm::vec3(resolution, resolution, resolution);
    this->maxNeighbourNum = maxNeighbourNum;
    this->gridSpacing = gridSpacing;
    this->gridOrigin = gridOrigin;
    this->totalParticles = totalParticles;
    this->fluidParticles = fluidParticles;
    this->boundaryParticleNum = totalParticles - fluidParticles;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNumPerBinSSBO);
    std::vector<GLuint> defaultParticleNumPerBin(binCount, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * binCount, defaultParticleNumPerBin.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleNumPerBinSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binIndexForParticleSSBO);
    std::vector<GLuint> defaultBinIndexPerParticle(totalParticles, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * totalParticles, defaultBinIndexPerParticle.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, binIndexForParticleSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixForBinReorderSSBO);
    std::vector<GLuint> initialPrefix(binCount + 1, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * (binCount + 1), initialPrefix.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, prefixForBinReorderSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesOrderedByBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * totalParticles, defaultBinIndexPerParticle.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, particlesOrderedByBinSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixIndexCounterSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * binCount, defaultParticleNumPerBin.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, prefixIndexCounterSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourListSSBO);
    std::vector<GLuint> defaultNeighbourList(fluidParticles * maxNeighbourNum, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * fluidParticles * maxNeighbourNum, defaultNeighbourList.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, neighbourListSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, endIndexNeighbourSSBO);
    std::vector<GLuint> defaultEndIndex(fluidParticles, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * fluidParticles, defaultEndIndex.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, endIndexNeighbourSSBO);

    // computing the neighbouring bin indices for all bins for flatNeighbours - only need to compute once
    std::vector<GLuint> flatNeighbours(binCount * 27, 111111);

    for (unsigned int x = 0; x < resolution; ++x) {
        for (unsigned int y = 0; y < resolution; ++y) {
            for (unsigned int z = 0; z < resolution; ++z) {

                int binIndex = x + y * resolution + z * resolution * resolution;
                int neighbourBase = binIndex * 27;
                int n = 0;

                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {

                            int nx = static_cast<int>(x) + dx;
                            int ny = static_cast<int>(y) + dy;
                            int nz = static_cast<int>(z) + dz;

                            if (nx >= 0 && nx < static_cast<int>(resolution) &&
                                ny >= 0 && ny < static_cast<int>(resolution) &&
                                nz >= 0 && nz < static_cast<int>(resolution)) {

                                GLuint neighbourIdx = nx + ny * resolution + nz * resolution * resolution;
                                flatNeighbours[neighbourBase + n] = neighbourIdx;
                            }
                            else {
                                flatNeighbours[neighbourBase + n] = 111111;
                            }

                            ++n;
                        }
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flatNeighboursSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * flatNeighbours.size(), flatNeighbours.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, flatNeighboursSSBO);

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
    GLuint groupNumFluid = (fluidParticles + 255) / 256;

    glClearNamedBufferData(particleNumPerBinSSBO, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

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
    computePrefixSumShader->setUInt("particleNum", totalParticles);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // creating a particlesOrderedByBin buffer using prefix sum 
    reorderShader->use();
    reorderShader->setUInt("particleNum", totalParticles);
    glDispatchCompute(groupNumAll, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /** Build the neighbour list
    *    the idea is to only search for nearby particles in the surrounding 27 bins 
    */
    buildNeighbourListShader->use();
    buildNeighbourListShader->setFloat("searchRadius", searchRadius);
    buildNeighbourListShader->setUInt("maxNeighboursPerParticle", maxNeighbourNum);
    buildNeighbourListShader->setUInt("fluidParticles", fluidParticles);
    buildNeighbourListShader->setUInt("boundaryParticleNum", boundaryParticleNum);
    buildNeighbourListShader->setUInt("totalParticles", totalParticles);
    glDispatchCompute(groupNumFluid, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
