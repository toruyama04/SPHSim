#include "utils.h"


float quadVertices[] = {
        -0.01f, -0.01f, 0.0f,
         0.01f, -0.01f, 0.0f,
        -0.01f,  0.01f, 0.0f,
         0.01f,  0.01f, 0.0f
};

int indexes[] = {
    0, 1, 2,
    1, 3, 2
};

float floorVertices[] = {
    // positions
    -10.0f, 0.0f, -10.0f,
     10.0f, 0.0f, -10.0f,
    -10.0f, 0.0f,  10.0f,
     10.0f, 0.0f,  10.0f
};

glm::vec3 fireworkPos[2] = {
    glm::vec3(0.0f, 1.0f, 7.0f),
    glm::vec3(5.0f, 8.0f, -5.0f)
};