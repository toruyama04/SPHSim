#include <glad/glad.h>

#include "Application.h"
#include "Firework.h"
#include "utils.h"


int main() {

    Application myApp(1920, 1080, "Firework");
    Firework myFirework;

    myFirework.addFirework(fireworkPos[0]);
    myApp.addFirework(&myFirework);

    myApp.run();

    return 0;
}

