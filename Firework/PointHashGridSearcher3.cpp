#include "PointHashGridSearcher3.h"


PointHashGridSearcher3::PointHashGridSearcher3(size_t resolutionX, size_t resolutionY, size_t resolutionZ, 
    double gridSpacing, unsigned int particleNum, GLuint bindingPoint)
{
    // particleNumPerBinSSBO -> count: bin num (resolution), value: number of particles per bin RESET
    // binIndexForParticleSSBO -> count: particle num (particleNum), value: bin index particle is in
    // prefixForBinReorderSSBO -> count: bin number (resolution), value: finding start index for bins for particlesOrderedByBinSSBO RESET
    // particleOrderedByBinSSBO -> count: particle num, value: particle indices ordered by bins RESET
    // tempBinOffsetSSBO -> count: bin number, value: used for counting index for same bins RESET
    // prefixForNeighbourListSSBO -> count: particle num, value: the number of valid nearby particles for each particle RESET
    // neighbourBinsIndicesForBinsSSBO -> count: bin number * potential bin neighbour, value: all neighbour bins for each bin NORESET
    // totalneigbourListSSBO -> count: single value, value: stores the right index to insert the nearby particle in neighbourList RESET
    // neighbourListSSBO -> count: valid nearby particles per particle, value: the indices of nearby particles RESET
    // startIndicesForNearbySSBO -> count: particle num, value: the start index number for nearby particles for neighbourListSSBO RESET

	glGenBuffers(1, &particleNumPerBinSSBO);
    glGenBuffers(1, &binIndexForParticleSSBO);
    glGenBuffers(1, &prefixForBinReorderSSBO);
    glGenBuffers(1, &particlesOrderedByBinSSBO);
    glGenBuffers(1, &tempBinOffsetSSBO);
    glGenBuffers(1, &prefixForNeighbourListSSBO);
    glGenBuffers(1, &neighbourBinsIndicesForBinSSBO);
    glGenBuffers(1, &totalNeigbourListSSBO);
    glGenBuffers(1, &neighbourListSSBO);
    glGenBuffers(1, &startIndicesForNearbySSBO);

    // 8
    std::vector<GLuint> initialOffsetP(particleNum, 0);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, prefixForNeighbourListSSBO);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * particleNum, initialOffsetP.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingPoint, prefixForNeighbourListSSBO);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // 9
    resolution = resolutionX * resolutionY * resolutionZ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNumPerBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, resolution * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 1, particleNumPerBinSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 10
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, binIndexForParticleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 2, binIndexForParticleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 11
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixForBinReorderSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * resolution, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 3, prefixForBinReorderSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 12
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesOrderedByBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 4, particlesOrderedByBinSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 13
    std::vector<GLuint> initialOffset(resolution, 0);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, tempBinOffsetSSBO);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * particleNum, initialOffset.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingPoint + 5, tempBinOffsetSSBO);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // 14
    std::vector<std::vector<std::vector<std::vector<GLuint>>>> neighbors(
        resolutionX, std::vector<std::vector<std::vector<GLuint>>>(
            resolutionY, std::vector<std::vector<GLuint>>(
                resolutionZ, std::vector<GLuint>(27, -1) // Initialize all neighbors with -1
            )
        )
    );
    for (unsigned int x = 0; x < resolutionX; ++x) {
        for (unsigned int y = 0; y < resolutionY; ++y) {
            for (unsigned int z = 0; z < resolutionZ; ++z) {
                // Index for neighbors (3x3x3 cube around the current cell)
                int index = 0;

                // Iterate through all neighbors including the cell itself
                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {
                            // Compute the neighbor's position
                            int nx = x + dx;
                            int ny = y + dy;
                            int nz = z + dz;

                            // Check if the neighbor is within bounds
                            if (nx >= 0 && nx < resolutionX &&
                                ny >= 0 && ny < resolutionY &&
                                nz >= 0 && nz < resolutionZ) {

                                // Calculate the 1D index for the neighbor
                                GLuint neighborIndex = nx + (ny * resolutionX) + (nz * resolutionX * resolutionY);
                                neighbors[x][y][z][index] = neighborIndex;
                            }
                            else {
                                // Out of bounds, keep it as -1
                                neighbors[x][y][z][index] = -1;
                            }

                            ++index; // Increment neighbor index
                        }
                    }
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourBinsIndicesForBinSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * resolution * 27, neighbors.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 6, neighbourBinsIndicesForBinSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 15
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbourListSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * particleNum * 32, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 7, neighbourListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 16
    GLuint defaultPrefix = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, totalNeigbourListSSBO);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &defaultPrefix, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingPoint + 8, tempBinOffsetSSBO);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // 17
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, startIndicesForNearbySSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * particleNum, initialOffsetP.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint + 9, startIndicesForNearbySSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    countShader = std::make_unique<Shader>("count.comp");
    prefixShader = std::make_unique<Shader>("prefix.comp");
    reorderShader = std::make_unique<Shader>("reorder.comp");
    generateNeighbourListShader = std::make_unique<Shader>("buildNeighbourList.comp");
    neighbourListStartIndicesShader = std::make_unique<Shader>("neighbourIndices.comp");
}


void PointHashGridSearcher3::build(GLuint particleNum)
{
    // count particles in each bin
    countShader->use();
    countShader->setVec3("gridResolution", glm::uvec3(25, 50, 25));
    countShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    countShader->setFloat("gridSpacing", 1.0f);
    countShader->setUInt("particleNum", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // calculate new index of bins based on particleNumPerBinSSBO
    prefixShader->use();
    prefixShader->setUInt("numBins", resolution);
    glDispatchCompute((resolution + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // reorder based on bin number in particlesOrderedByBinSSBO
    reorderShader->use();
    reorderShader->setUInt("numParticles", particleNum);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

    // building the neighbour list
    generateNeighbourListShader->use();
    generateNeighbourListShader->setVec3("gridResolution", glm::uvec3(25, 50, 25));
    generateNeighbourListShader->setVec3("gridOrigin", glm::vec3(0, 0, 0));
    generateNeighbourListShader->setFloat("gridSpacing", 1.0f);
    generateNeighbourListShader->setUInt("particleNum", particleNum);
    generateNeighbourListShader->setFloat("smoothingLength", 1.0f);
    glDispatchCompute((particleNum + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

    // getting start indices for particle neighbours
    neighbourListStartIndicesShader->use();
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

