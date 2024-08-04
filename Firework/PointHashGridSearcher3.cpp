#include "PointHashGridSearcher3.h"


PointHashGridSearcher3::PointHashGridSearcher3(size_t resolutionX, size_t resolutionY, size_t resolutionZ, 
    double gridSpacing, unsigned int particleNum, GLuint bindingPoint)
{
	glGenBuffers(1, &binsSSBO);
	glGenBuffers(1, &neighboursSSBO);
    glGenBuffers(1, &particleIndicesSSBO);
    glGenBuffers(1, &prefixSumSSBO);
    glGenBuffers(1, &reorderedSSBO);
    glGenBuffers(1, &tempBinOffsetSSBO);

    // 8
    const int maxNeighbours = 32;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighboursSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleNum * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, neighboursSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 9
    size_t numGridCells = resolutionX * resolutionY * resolutionZ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numGridCells * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 1, binsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 10
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 2, particleIndicesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 11
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixSumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * numGridCells, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 3, prefixSumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 12
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, reorderedSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 4, reorderedSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 13
    std::vector<GLuint> initialOffset(numGridCells, 0);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, tempBinOffsetSSBO);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * particleNum, initialOffset.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingPoint + 6, tempBinOffsetSSBO);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    countShader = std::make_unique<Shader>("count.comp");
    prefixShader = std::make_unique<Shader>("prefix.comp");
    reorderShader = std::make_unique<Shader>("reorder.comp");
    resolution = resolutionX * resolutionY * resolutionZ;
}

void PointHashGridSearcher3::fillNeighbourList()
{
	// for each particle, we find their 8 closest cells
    // for each particle in those cells, find the distance from origin particle and add to neighbourlist
    //      have a prefix sum for the number of neighbours per particle, increase per neighbour
    // using prefix sum index for the particle index, perform functions.
}


void PointHashGridSearcher3::build(GLuint particleNum)
{
	// reset bins
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binsSSBO);
    std::vector<GLuint> resetData(resolution, 0);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _resolution.x * _resolution.y * _resolution.z * sizeof(GLuint), resetData.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // count particles in bin
    countShader->use();
    countShader->setVec3("gridResolution", glm::uvec3(25, 50, 25));
    countShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    countShader->setFloat("gridSpacing", 1.0f);
    countShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // calculate new index
    prefixShader->use();
    prefixShader->setUInt("numBins", resolution);
    glDispatchCompute((resolution + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // reorder based on bin number
    reorderShader->use();
    reorderShader->setUInt("numParticles", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
}

void PointHashGridSearcher3::forEachNearbyPoint(const glm::vec3& origin, double radius, const std::function<void(size_t, const glm::vec3&)>& callback) const
{
	// first find the closest 8 buckets given origin
    // get the number of points inside that bucket
    // for each point:
    //      get the distance from that point to origin squared
    //      compare the squared length with the squared radius
    //      call the function on that point 
}

