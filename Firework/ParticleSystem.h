#pragma once

#include <glm/glm.hpp>

#include <vector>


class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();


private:

    unsigned int particleCount;
    float currentTime;

    std::vector<glm::vec4> particles;
    std::vector<glm::vec4> velocity;
    std::vector<glm::vec4> colour;
    std::vector<glm::vec4> regionPoint;
    std::vector<float> lifetime;
    std::vector<float> swirl;
    std::vector<float> fadeRate;
    std::vector<glm::vec4> origin;

};
