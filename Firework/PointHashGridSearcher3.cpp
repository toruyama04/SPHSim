#include "PointHashGridSearcher3.h"
#include <vector>

PointHashGridSearcher3::PointHashGridSearcher3(GLuint resolutionX, GLuint resolutionY, GLuint resolutionZ, unsigned int particleNum, GLuint maxneighbourNum)
{
    glGenBuffers(1, &particleNumPerBinSSBO);
    glGenBuffers(1, &binIndexForParticleSSBO);
    glGenBuffers(1, &prefixForBinReorderSSBO);
    glGenBuffers(1, &particlesOrderedByBinSSBO);
    glGenBuffers(1, &flatNeighboursSSBO);
    glGenBuffers(1, &prefixIndexCounter);
    glGenBuffers(1, &neighbourListSSBO);
    glGenBuffers(1, &endIndexNeighbourSSBO);

    resolutionVec = glm::vec3(resolutionX, resolutionY, resolutionZ);
    binCount = resolutionX * resolutionY * resolutionZ;
    this->maxneighbourNum = maxneighbourNum;

    std::vector<GLuint> initialBinValue(binCount, 0);
    std::vector<GLuint> initialParticleValue(particleNum, 0);
    std::vector<GLuint> initialN(particleNum * maxneighbourNum, 0);

    // Initialize buffers with data
    glNamedBufferData(particleNumPerBinSSBO, sizeof(GLuint) * binCount, initialBinValue.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(binIndexForParticleSSBO, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(prefixForBinReorderSSBO, sizeof(GLuint) * binCount, nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferData(particlesOrderedByBinSSBO, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);

    std::vector<GLuint> flatNeighbors(resolutionX * resolutionY * resolutionZ * 27, 111111);
    for (unsigned int x = 0; x < resolutionX; ++x) {
        for (unsigned int y = 0; y < resolutionY; ++y) {
            for (unsigned int z = 0; z < resolutionZ; ++z) {
                int baseIndex = (x + (y * resolutionX) + (z * resolutionX * resolutionY)) * 27;
                int index = 0;
                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            int nx = x + dx;
                            int ny = y + dy;
                            int nz = z + dz;
                            if (nx >= 0 && nx < resolutionX &&
                                ny >= 0 && ny < resolutionY &&
                                nz >= 0 && nz < resolutionZ) {
                                GLuint neighborIndex = nx + (ny * resolutionX) + (nz * resolutionX * resolutionY);
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

    glNamedBufferData(flatNeighboursSSBO, flatNeighbors.size() * sizeof(GLuint), flatNeighbors.data(), GL_STATIC_DRAW);
    glNamedBufferData(prefixIndexCounter, sizeof(GLuint) * binCount, initialBinValue.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(neighbourListSSBO, sizeof(GLuint) * particleNum * maxneighbourNum, initialN.data(), GL_DYNAMIC_DRAW);
    glNamedBufferData(endIndexNeighbourSSBO, sizeof(GLuint) * particleNum, initialParticleValue.data(), GL_DYNAMIC_DRAW);

    // Bind buffers to binding points
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, particleNumPerBinSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, binIndexForParticleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, prefixForBinReorderSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, particlesOrderedByBinSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, flatNeighboursSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, neighbourListSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, prefixIndexCounter);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, endIndexNeighbourSSBO);

    countShader = std::make_unique<Shader>("shaders/count.comp");
    reorderShader = std::make_unique<Shader>("shaders/reorder.comp");
    generateNeighbourListShader = std::make_unique<Shader>("shaders/buildNeighbourList.comp");
    /*pass1 = std::make_unique<Shader>("shaders/pass1prefix.comp");
    pass2 = std::make_unique<Shader>("shaders/pass2prefix.comp");
    pass3 = std::make_unique<Shader>("shaders/pass3prefix.comp");*/
}

void PointHashGridSearcher3::build(GLuint particleNum, float searchRadius) {
    std::vector<GLuint> initialOffset(binCount, 0);

    glNamedBufferSubData(particleNumPerBinSSBO, 0, binCount * sizeof(GLuint), initialOffset.data());
    glNamedBufferSubData(prefixIndexCounter, 0, binCount * sizeof(GLuint), initialOffset.data());

    // Count particles in each bin, assign bin to each particle
    countShader->use();
    countShader->setVec3("gridResolution", resolutionVec);
    countShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    countShader->setFloat("gridSpacing", 1.0f);
    countShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Retrieve particle counts per bin
    std::vector<GLuint> particleNumPerBin(binCount);
    glGetNamedBufferSubData(particleNumPerBinSSBO, 0, binCount * sizeof(GLuint), particleNumPerBin.data());

    // Compute prefix sum on CPU
    std::vector<GLuint> prefixSum(binCount, 0);
    GLuint sum = 0;
    for (GLuint i = 0; i < binCount; ++i) {
        prefixSum[i] = sum;
        sum += particleNumPerBin[i];
    }

    // Write prefix sum back to GPU
    glNamedBufferSubData(prefixForBinReorderSSBO, 0, binCount * sizeof(GLuint), prefixSum.data());
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /*pass1->use();
    glDispatchCompute((resolution + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    pass2->use();
    glDispatchCompute((resolution + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    pass3->use();
    glDispatchCompute((resolution + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);*/

    // Reorder based on bin number in particlesOrderedByBinSSBO
    reorderShader->use();
    reorderShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Build the neighbor list
    generateNeighbourListShader->use();
    generateNeighbourListShader->setFloat("searchRadius", 1.0f);
    generateNeighbourListShader->setUInt("maxNeighborsPerParticle", maxneighbourNum);
    generateNeighbourListShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
