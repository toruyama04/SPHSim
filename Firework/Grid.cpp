#include "Grid.h"
#include <vector>

Grid::Grid(glm::vec3 gridOrigin, glm::vec3 extents, GLuint particleNum,
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

    // should precompute instead of fixed value
    // GLuint resolution = (extents.x - gridOrigin.x) / gridSpacing;
    GLuint resolution = 25;
    binCount = resolution * resolution * resolution;
    resolutionVec = glm::vec3(resolution, resolution, resolution);
    this->maxNeighbourNum = maxNeighbourNum;
    this->gridSpacing = gridSpacing;
    this->gridOrigin = gridOrigin;

    std::vector<GLuint> initialBinValue(binCount, 0);
    std::vector<GLuint> initialParticleValue(particleNum, 0);
    std::vector<GLuint> initialN(particleNum * maxNeighbourNum, 0);
    std::vector<GLuint> initialPrefix(binCount + 1, 0);
    initialPrefix[binCount] = particleNum;

    // Initialize buffers with data
    glNamedBufferData(particleNumPerBinSSBO, sizeof(GLuint) * binCount, initialBinValue.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(binIndexForParticleSSBO, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(prefixForBinReorderSSBO, sizeof(GLuint) * (binCount + 1), initialPrefix.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(particlesOrderedByBinSSBO, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);

    // computing the neighbouring bin indices for all bins
    // const GLuint NO_NEIGHBOR = std::numeric_limits<GLuint>::max();
    std::vector<GLuint> flatNeighbors(binCount * 27, 111111);
    for (unsigned int x = 0; x < resolution; ++x) {
        for (unsigned int y = 0; y < resolution; ++y) {
            for (unsigned int z = 0; z < resolution; ++z) {
                int baseIndex = (x + (y * resolution) + (z * resolution * resolution)) * 27;
                int index = 0;
                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            unsigned int nx = x + dx;
                            unsigned int ny = y + dy;
                            unsigned int nz = z + dz;
                            if (nx >= 0 && nx < resolution &&
                                ny >= 0 && ny < resolution &&
                                nz >= 0 && nz < resolution) {
                                GLuint neighborIndex = nx + (ny * resolution) + (nz * resolution * resolution);
                                flatNeighbors[baseIndex + index] = neighborIndex;
                            }
                            else {
                                flatNeighbors[baseIndex + index] = 111111;
                            }
                            ++index;
                        }
                    }
                }
            }
        }
    }

    glNamedBufferData(flatNeighboursSSBO, sizeof(GLuint) * flatNeighbors.size(), flatNeighbors.data(), GL_STATIC_DRAW);
    glNamedBufferData(prefixIndexCounterSSBO, sizeof(GLuint) * binCount, initialBinValue.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(neighbourListSSBO, sizeof(GLuint) * particleNum * maxNeighbourNum, nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(endIndexNeighbourSSBO, sizeof(GLuint) * particleNum, initialParticleValue.data(), GL_DYNAMIC_DRAW);

    // Bind buffers to binding points
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, particleNumPerBinSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, binIndexForParticleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, prefixForBinReorderSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, particlesOrderedByBinSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, flatNeighboursSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, neighbourListSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, prefixIndexCounterSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, endIndexNeighbourSSBO);

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

void Grid::build(GLuint particleNum, float searchRadius) {
    // we dispatch 256 threads in x for a compute shader over all particles
    GLuint groupNum = (particleNum + 255) / 256;

    glClearNamedBufferData(particleNumPerBinSSBO, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

    // setting bin index for each particle, counting particles per bin
    countShader->use();
    countShader->setVec3("gridResolution", resolutionVec);
    countShader->setVec3("gridOrigin", gridOrigin);
    countShader->setFloat("gridSpacing", gridSpacing);
    countShader->setUInt("particleNum", particleNum);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // On CPU
    {
        std::vector<GLuint> particleNumPerBin(binCount);
        glGetNamedBufferSubData(particleNumPerBinSSBO, 0, binCount * sizeof(GLuint), particleNumPerBin.data());

        // Compute prefix sum on CPU
        std::vector<GLuint> prefixSum(binCount + 1, 0);
        GLuint sum = 0;
        for (GLuint i = 0; i < binCount; ++i) {
            prefixSum[i] = sum;
            sum += particleNumPerBin[i];
        }
        prefixSum[binCount] = particleNum;
        // Write prefix sum back to GPU
        glNamedBufferSubData(prefixForBinReorderSSBO, 0, (binCount + 1) * sizeof(GLuint), prefixSum.data());
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // Computing prefix indices on GPU
    computePrefixSumShader->use();
    computePrefixSumShader->setUInt("binCount", binCount);
    computePrefixSumShader->setUInt("particleNum", particleNum);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // creating a particlesOrderedByBin buffer using prefix sum 
    reorderShader->use();
    reorderShader->setUInt("particleNum", particleNum);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /** Build the neighbor list
    *    the idea is to only search for nearby particles in the surrounding 27 bins 
    */
    buildNeighbourListShader->use();
    buildNeighbourListShader->setFloat("searchRadius", searchRadius);
    buildNeighbourListShader->setUInt("maxNeighborsPerParticle", maxNeighbourNum);
    buildNeighbourListShader->setUInt("particleNum", particleNum);
    glDispatchCompute(groupNum, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
