#include <glad/glad.h>
#include "Firework.h"
#include "utils.h"


int main() {

    Application myApp;
    Firework myFirework;

    myFirework.addFirework(fireworkPos[0]);
    myApp.addFirework(myFirework);

    myApp.run();

    return 0;
}

