#include "PointHashGridSearcher3.h"

#include <vector>


PointHashGridSearcher3::PointHashGridSearcher3(GLuint resolutionX, GLuint resolutionY, GLuint resolutionZ,
                                               double gridSpacing, unsigned int particleNum)
{
    // particleNumPerBinSSBO -> count: bin num (resolution), value: number of particles per bin RESET
    // binIndexForParticleSSBO -> count: particle num (particleNum), value: bin index particle is in
    // prefixForBinReorderSSBO -> count: bin number (resolution), value: finding start index for bins for particlesOrderedByBinSSBO RESET
    // particleOrderedByBinSSBO -> count: particle num, value: particle indices ordered by bins RESET
    // tempBinOffsetSSBO -> count: bin number, value: used for counting index for same bins RESET
    // prefixForNeighbourListSSBO -> count: particle num, value: the number of valid nearby particles for each particle RESET
    // neighbourBinsIndicesForBinsSSBO -> count: bin number * potential bin neighbour, value: all neighbour bins for each bin NORESET
    // totalNeigbourListACB -> count: single value, value: stores the right index to insert the nearby particle in neighbourList RESET
    // neighbourListSSBO -> count: valid nearby particles per particle, value: the index of nearby particles RESET
    // startIndicesForNearbySSBO -> count: particle num, value: the start index number for nearby particles for neighbourListSSBO RESET

	glGenBuffers(1, &particleNumPerBinSSBO);
    glGenBuffers(1, &binIndexForParticleSSBO);
    glGenBuffers(1, &prefixForBinReorderSSBO);
    glGenBuffers(1, &particlesOrderedByBinSSBO);
    glGenBuffers(1, &groupSumSSBO);
    glGenBuffers(1, &groupPrefixSumSSBO);
    glGenBuffers(1, &flatNeighboursSSBO);
    // glGenBuffers(1, &totalNeigbourListACB);
    glGenBuffers(1, &neighbourListSSBO);
    glGenBuffers(1, &neighbourOffsetSSBO);
    glGenBuffers(1, &indexInsertToBinSSBO);

    _resolution = glm::vec3(resolutionX, resolutionY, resolutionZ);
    resolution = resolutionX * resolutionY * resolutionZ;
    std::vector<GLuint> initialOffset(resolution, 0);
    std::vector<GLuint> initialOffsetP(particleNum, 0);

    // 9
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNumPerBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, resolution * sizeof(GLuint), initialOffset.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, particleNumPerBinSSBO);

    // 10
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binIndexForParticleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, binIndexForParticleSSBO);

    // 11
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixForBinReorderSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * resolution, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, prefixForBinReorderSSBO);

    // 12
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesOrderedByBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, particlesOrderedByBinSSBO);

    // 13 - groupSumsSSBO - cumulative sum for each workgroup
    GLuint numGroups = (resolution + 255) / 256;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, groupSumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * numGroups, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, groupSumSSBO);

    // 14
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, groupPrefixSumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * numGroups, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, groupPrefixSumSSBO);

    // 15
    std::vector<GLuint> flatNeighbors(resolutionX * resolutionY * resolutionZ * 27, -1);

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
                                flatNeighbors[baseIndex + index] = -1;
                            }

                            ++index;
                        }
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flatNeighboursSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, flatNeighbors.size() * sizeof(GLuint), flatNeighbors.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, flatNeighboursSSBO);

    // 16
    /*GLuint defaultPrefix = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, totalNeigbourListACB);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &defaultPrefix, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 16, totalNeigbourListACB);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);*/

    // 17
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourListSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum * 100, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, neighbourListSSBO);

    // 18
    std::vector<GLuint> neighborOffsets(particleNum);
    for (GLuint i = 0; i < particleNum; ++i)
    {
        neighborOffsets[i] = i * 100;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, neighborOffsets.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 18, neighbourOffsetSSBO);

    // 19 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexInsertToBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * resolution, initialOffset.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 19, indexInsertToBinSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    countShader = std::make_unique<Shader>("shaders/count.comp");
    pass1 = std::make_unique<Shader>("shaders/pass1prefix.comp");
    pass2 = std::make_unique<Shader>("shaders/pass2prefix.comp");
    pass3 = std::make_unique<Shader>("shaders/pass3prefix.comp");
    reorderShader = std::make_unique<Shader>("shaders/reorder.comp");
    generateNeighbourListShader = std::make_unique<Shader>("shaders/buildNeighbourList.comp");
}

void PointHashGridSearcher3::build(GLuint particleNum)
{
    // reset buffers that need to be reset here 
    std::vector<GLuint> initialOffsetP(particleNum, 0);
    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixForNeighbourListSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particleNum * sizeof(GLuint), initialOffsetP.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

    std::vector<GLuint> initialOffset(resolution, 0);
    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, tempBinOffsetSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, resolution * sizeof(GLuint), initialOffset.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

    std::vector<GLuint> initialN(particleNum * 32, -1.0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourListSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * particleNum * 32, initialN.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNumPerBinSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, resolution * sizeof(GLuint), initialOffset.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, startIndicesForNearbySSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * particleNum, initialOffsetP.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexInsertToBinSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * resolution, initialOffset.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // count particles in each bin
    countShader->use();
    countShader->setUVec3("gridResolution", _resolution);
    countShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    countShader->setFloat("gridSpacing", 1.0f);
    countShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, binIndexForParticleSSBO);
    GLuint* num = static_cast<GLuint*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    GLuint binIndex = num[0];
    std::cout << "Bin index for 0: " << binIndex << "\n";
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

    GLuint numGroups = (resolution + 255) / 256;

    // calculate new index of bins based on particleNumPerBinSSBO
    pass1->use();
    pass1->setUInt("numBins", resolution);
    glDispatchCompute(numGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    pass2->use();
    pass2->setUInt("numGroups", numGroups);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    pass3->use();
    pass3->setUInt("numBins", resolution);
    glDispatchCompute(numGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // reorder based on bin number in particlesOrderedByBinSSBO
    reorderShader->use();
    reorderShader->setUInt("numParticles", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // building the neighbour list
    /*generateNeighbourListShader->use();
    generateNeighbourListShader->setVec3("gridResolution", _resolution);
    generateNeighbourListShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    generateNeighbourListShader->setFloat("gridSpacing", 1.0f);
    generateNeighbourListShader->setUInt("particleNum", particleNum);
    generateNeighbourListShader->setFloat("smoothingLength", 1.0f);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);*/
}

