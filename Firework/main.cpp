#include <glad/glad.h>
#include "Application.h"
#include "Firework.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>


glm::vec3 fireworkPos[2] = {
    // check grid sizes for neighbour search
    glm::vec3(5.0f, 8.0f, 5.0f),
    glm::vec3(5.0f, 8.0f, -5.0f)
};

int main() {

    Application* myApp = new Application(1920, 1080, "Firework");
    Firework* myFirework = new Firework;

    // myFirework->addFirework(fireworkPos[0]);
    myFirework->addParticleCube(glm::vec3(2.0f, 3.0f, 2.0f), 0.6f, 5);
    myApp->addFirework(myFirework);
    myApp->run();

    return 0;
}
