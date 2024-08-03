#include "PointHashGridSearcher3.h"


PointHashGridSearcher3::PointHashGridSearcher3(size_t resolutionX, size_t resolutionY, size_t resolutionZ, 
    double gridSpacing, unsigned int particleNum, GLuint bindingPoint)
{
	glGenBuffers(1, &gridSSBO);
	glGenBuffers(1, &neighboursSSBO);
    glGenBuffers(1, &particleIndicesSSBO);

    const int maxNeighbours = 32;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighboursSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particleNum * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, neighboursSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    size_t numGridCells = resolutionX * resolutionY * resolutionZ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numGridCells * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 1, gridSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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

