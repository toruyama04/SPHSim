#include <glad/glad.h>
#include "Application.h"
#include "Firework.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

////
glm::vec3 fireworkPos[2] = {
    // check grid sizes for neighbour search
    glm::vec3(5.0f, 8.0f, 5.0f),
    glm::vec3(5.0f, 8.0f, -5.0f)
};

int main() {

    Application* myApp = new Application(1920, 1080, "Firework");
    Firework* myFirework = new Firework;

    // myFirework->addFirework(fireworkPos[0]);
    myFirework->addParticleCube(glm::vec3(6.0, 5.0, 5.5), 0.6, 10);
    myApp->addFirework(myFirework);
    myApp->run();

    return 0;


}
////

////
/*#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <cmath>

// Constants
const float smoothingLength = 1.0f; // Neighbor search radius

// Define a struct for particle data
struct Particle {
    glm::vec3 position;
    std::vector<int> neighbors; // Store indices of neighbors
};

// Function to calculate the distance between two particles
float calculateDistance(const glm::vec3& posA, const glm::vec3& posB) {
    return glm::distance(posA, posB);
}

// Function to find neighbors for each particle
void findNeighbors(std::vector<Particle>& particles) {
    size_t numberOfParticles = particles.size();

    // Iterate through each particle to find its neighbors
    for (size_t i = 0; i < numberOfParticles; ++i) {
        Particle& currentParticle = particles[i];

        for (size_t j = 0; j < numberOfParticles; ++j) {
            if (i == j) continue; // Skip self

            // Calculate the distance between particles i and j
            float distance = calculateDistance(currentParticle.position, particles[j].position);

            // Check if the distance is within the smoothing length
            if (distance < smoothingLength) {
                currentParticle.neighbors.push_back(j); // Add particle j as a neighbor
            }
        }
    }
}

int main() {
    // Example particle setup, similar to your particle creation code
    std::vector<Particle> particles;
    glm::vec3 origin(6.0f, 5.0f, 5.5f);
    float spacing = 0.6f;
    int particlesPerSide = 10; // Small number for demo
    float halfSide = (particlesPerSide - 1) * spacing / 2.0f;
    float gap = 0.1f; // Define the gap between the two halves

    // Loop to create particles in the split cube (same logic as before)
    for (int x = 0; x < particlesPerSide; ++x) {
        for (int y = 0; y < particlesPerSide; ++y) {
            for (int z = 0; z < particlesPerSide; ++z) {
                // Calculate particle position
                glm::vec3 pos = origin + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                // Adjust the particle position based on whether it's in the left or right half
                if (pos.x < origin.x) {
                    pos.x -= gap / 2.0f;  // Shift left half particles further left by half the gap
                }
                else {
                    pos.x += gap / 2.0f;  // Shift right half particles further right by half the gap
                }

                // Add particle with position
                particles.push_back({ pos });
            }
        }
    }

    // Find neighbors for each particle
    findNeighbors(particles);

    // Output neighbor information
    for (size_t i = 0; i < particles.size(); ++i) {
        std::cout << "Particle " << i << " neighbors: ";
        for (int neighbor : particles[i].neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}*/
////

////
/*
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>

// Constants
const int gridResolutionX = 10;
const int gridResolutionY = 10;
const int gridResolutionZ = 10;
const float gridSpacing = 1.0f;
const glm::vec3 gridOrigin(0.0f, 0.0f, 0.0f);

struct Particle {
    glm::vec3 position;
    int binIndex;
};

// Function to calculate 1D bin index from 3D bin coordinates
int calculateBinIndex(int bx, int by, int bz) {
    return bx + by * gridResolutionX + bz * gridResolutionX * gridResolutionY;
}

// Function to calculate 3D bin coordinates from particle position
glm::ivec3 calculateBinCoordinates(const glm::vec3& position) {
    return glm::ivec3(
        std::floor((position.x - gridOrigin.x) / gridSpacing),
        std::floor((position.y - gridOrigin.y) / gridSpacing),
        std::floor((position.z - gridOrigin.z) / gridSpacing)
    );
}

// Main function
int main() {
    std::vector<Particle> particles;

    int particlesPerSide = 10;
    float spacing = 0.6f;
    float gap = 0.1f;
    float halfSide = (particlesPerSide - 1) * spacing / 2.0f;
    glm::vec3 origin(6.0f, 5.0f, 5.5f);

    // Create particles
    for (int x = 0; x < particlesPerSide; ++x) {
        for (int y = 0; y < particlesPerSide; ++y) {
            for (int z = 0; z < particlesPerSide; ++z) {
                // Calculate particle position
                glm::vec3 pos = origin + glm::vec3(x * spacing - halfSide, y * spacing - halfSide, z * spacing - halfSide);

                // Adjust the particle position based on whether it's in the left or right half
                if (pos.x < origin.x) {
                    pos.x -= gap / 2.0f;  // Shift left half particles further left by half the gap
                }
                else {
                    pos.x += gap / 2.0f;  // Shift right half particles further right by half the gap
                }

                Particle p;
                p.position = pos;

                // Calculate which bin the particle belongs to
                glm::ivec3 binCoords = calculateBinCoordinates(p.position);
                if (binCoords.x >= 0 && binCoords.x < gridResolutionX &&
                    binCoords.y >= 0 && binCoords.y < gridResolutionY &&
                    binCoords.z >= 0 && binCoords.z < gridResolutionZ) {
                    p.binIndex = calculateBinIndex(binCoords.x, binCoords.y, binCoords.z);
                    particles.push_back(p);
                }
                else {
                    std::cout << "Particle out of grid bounds!\n";
                }
            }
        }
    }

    // Store the particles in bins
    std::vector<std::vector<Particle>> bins(gridResolutionX * gridResolutionY * gridResolutionZ);

    for (const auto& p : particles) {
        bins[p.binIndex].push_back(p);
    }

    // Print particles in each bin
    for (int i = 0; i < bins.size(); ++i) {
        std::cout << "Bin " << i << " contains " << bins[i].size() << " particles.\n";
        for (const auto& p : bins[i]) {
            std::cout << "  Particle at position (" << p.position.x << ", " << p.position.y << ", " << p.position.z << ")\n";
        }
    }

    return 0;
}
*/
////
