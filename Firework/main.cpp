#include <glad/glad.h>
#include "Application.h"
#include "Firework.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>


glm::vec3 fireworkPos[2] = {
    glm::vec3(0.0f, 1.0f, 7.0f),
    glm::vec3(5.0f, 8.0f, -5.0f)
};

int main() {

    Application* myApp = new Application(1920, 1080, "Firework");
    Firework* myFirework = new Firework;

    myFirework->addFirework(fireworkPos[0]);
    myApp->addFirework(myFirework);
    myApp->run();

    return 0;
}

