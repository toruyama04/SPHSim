#include <glad/glad.h>

#include "Application.h"
#include "Firework.h"
#include "utils.h"


int main() {

    Application* myApp = new Application(1920, 1080, "Firework");
    Firework* myFirework = new Firework;

    myFirework->addFirework(fireworkPos[0]);
    myApp->addFirework(myFirework);

    myApp->run();

    return 0;
}

