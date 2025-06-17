#include <glad/glad.h>
#include "Application.h"
#include "SPHSim.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>



int main() {

    Application* myApp = new Application(1920, 1080, "Firework");
    Sim* mySim = new Sim;

    mySim->addParticleCube(glm::vec3(5.0f, 5.0f, 5.0f), 0.4f, 12);
    myApp->addSim(mySim);
    myApp->run();

    return 0;
}
