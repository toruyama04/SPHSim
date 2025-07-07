#include <glad/glad.h>
#include "Application.h"
#include "SPHSim.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>



int main() {

    Application* myApp = new Application(1920, 1080, "SPHSim");
    Sim* mySim = new Sim;

    mySim->addParticleCube(glm::vec3(2.5f, 2.5f, 2.5f), 0.1f, 35, glm::vec3(5.0f, 5.0f, 5.0f));
    myApp->addSim(mySim);
    myApp->run();

    return 0;
}
