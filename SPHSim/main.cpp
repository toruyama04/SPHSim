#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "Application.h"
#include "SPHSim.h"



int main() {

    // Create a new OpenGL application, add simulation, and run
    Application* myApp = new Application(1920, 1080, "SPHSim");
    Sim* mySim = new Sim(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(4.0f, 4.0f, 4.0f));

    mySim->addParticleCube(glm::vec3(2.0f, 2.0f, 2.0f), 0.14f, 20);
    myApp->addSim(mySim);
    myApp->run();

    delete myApp;
    return 0;
}
