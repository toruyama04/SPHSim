#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "shader.h"
#include "camera.h"

#include <vector>
#include <random>
#include <iostream>
#include <array>

struct Particle {
    glm::vec4 position;
    glm::vec4 velocity;
    glm::vec4 alpha;
    glm::vec4 regionPoint;
    float lifetime;
    float swirl;
    float fadeRate;
    float originY;
    glm::vec4 origin;
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
GLuint updateParticles(float deltaTime, unsigned int SSBO, unsigned int ACB);
void createFirework(std::vector<Particle>& particles, const glm::vec4& position, int count);
void displayFPS(GLFWwindow* window);
glm::vec4 regionCheck(const glm::vec4& v, const glm::vec4& origin);
Particle defaultParticle();

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float totaltime = 0.0f;
bool done1 = false;
bool done2 = false;

const unsigned int maxParticles = 20000;
const unsigned int particleNum = 10000;
const unsigned int fireworkNum = 1;
GLuint lastUsedId = 0;
GLuint SSBOGlobal;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Firework", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // global states
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

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

    //float fireworkRocket[] = {
    //    -3.0f, -5.0f, 0.0f,
    //     3.0f, -5.0f, 0.0f,
    //    -3.0f,  5.0f, 0.0f,
    //     3.0f,  5.0f, 0.0f
    //};

    glm::vec4 fireworkPos[2] = {
        glm::vec4(0.0f, 5.0f, 1.0f, 1.0f),
        glm::vec4(5.0f, 8.0f, -5.0f, 1.0f)
    };

    std::vector<Particle> particles(maxParticles, defaultParticle());

    for (unsigned int i = 0; i < fireworkNum; i++) {
        createFirework(particles, fireworkPos[i], particleNum);
    }

    /* firework particles */
    unsigned int VAO, VBO, EBO, SSBO, ACB;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &SSBO);
    SSBOGlobal = SSBO;
    glGenBuffers(1, &ACB);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * maxParticles, particles.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

    /*Particle* debugBuffer = new Particle[maxParticles];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * maxParticles, debugBuffer);
    for (int i = 0; i < maxParticles; ++i)
    {
        std::cout << "Particle " << i << " lifetime: " << debugBuffer[i].lifetime << std::endl;
    }*/

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ACB);

    /* floor */
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // shaders
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag");
    Shader computeShader("shaders/particle.comp");

    Shader floorShader("shaders/floor.vert", "shaders/floor.frag");
    floorShader.use();
    floorShader.setVec3("colour", glm::vec3(0.5f, 0.5f, 0.5f));

    std::cout << "start\n";
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        totaltime += deltaTime;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        computeShader.use();
        computeShader.setFloat("deltaTime", deltaTime);
        computeShader.setVec3("grav", glm::vec3(0.0f, -5.81f, 0.0));
        computeShader.setFloat("maxLife", 3.0f);
        computeShader.setFloat("dampingFactor", 0.0005);
        computeShader.setFloat("minVelocity", 0.2);
        computeShader.setFloat("attractionStrength", 0.75f);
        computeShader.setFloat("spiralness", 1.4);
        computeShader.setFloat("spiralAttractionStrength", 0.4);
        computeShader.setVec3("acceleration", glm::vec3(1.3f));

        /*GLuint num;
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &num);
        std::cout << "atomic counter: " << num << std::endl;*/

		GLuint numToDraw = updateParticles(totaltime, SSBO, ACB);
        // std::cout << "acb: " << numToDraw << std::endl;

        particleShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        particleShader.setMat4("model", model);
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numToDraw);

        floorShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f));
        floorShader.setMat4("model", model);
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);

        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);

        displayFPS(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// function to update particles
GLuint updateParticles(float totalTime, unsigned int SSBO, unsigned int ACB) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
    GLuint initial = 0;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &initial);
    glDispatchCompute((maxParticles + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

    GLuint numAliveParticles;
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &numAliveParticles);

	Particle* particleData = (Particle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(Particle), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    if (totaltime > 4.0f && done1 == false)
    {
        for (int i = 0; i < particleNum; ++i)
        {
            particleData[i].position = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
            float theta = glm::linearRand(0.0f, glm::two_pi<float   >());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(5.0f, 10.0f);
            particleData[i].velocity.x = speed * sin(phi) * cos(theta);
            particleData[i].velocity.y = speed * sin(phi) * sin(theta);
            particleData[i].velocity.z = speed * cos(phi);
            particleData[i].velocity.w = 1.0f;
            particleData[i].regionPoint = regionCheck(glm::normalize(particleData[i].velocity), glm::vec4(0.0f, 5.0f, 1.0f, 1.0f));
            if (particleData[i].regionPoint != glm::vec4(0.0f, 5.0f, 1.0f, 1.0f))
                particleData[i].swirl = 1.0f;
            else
                particleData[i].swirl = 0.0f;
            particleData[i].alpha = glm::vec4(1.0f);
            particleData[i].lifetime = 3.0f;
            particleData[i].fadeRate = 1.0f / 3.0f;
            particleData[i].originY = 5.0f;
            particleData[i].origin = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
            done1 = true;
        }
    } else if (totaltime > 8.0f && done2 == false)
    {
        for (int i = 0; i < particleNum; ++i)
        {
            particleData[i].position = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
            float theta = glm::linearRand(0.0f, glm::two_pi<float   >());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(5.0f, 10.0f);
            particleData[i].velocity.x = speed * sin(phi) * cos(theta);
            particleData[i].velocity.y = speed * sin(phi) * sin(theta);
            particleData[i].velocity.z = speed * cos(phi);
            particleData[i].velocity.w = 1.0f;
            particleData[i].regionPoint = regionCheck(glm::normalize(particleData[i].velocity), glm::vec4(0.0f, 5.0f, 1.0f, 1.0f));
            if (particleData[i].regionPoint != glm::vec4(0.0f, 5.0f, 1.0f, 1.0f))
                particleData[i].swirl = 1.0f;
            else
                particleData[i].swirl = 0.0f;
            particleData[i].alpha = glm::vec4(1.0f);
            particleData[i].lifetime = 3.0f;
            particleData[i].fadeRate = 1.0f / 3.0f;
            particleData[i].originY = 5.0f;
            particleData[i].origin = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
            done2 = true;
        }
    }
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    /*lastUsedId = numAliveParticles;

    if (particleData != NULL)
    {
        if (particleData[0].lifetime > 3.0f) {
            
        }
        
    }*/

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return numAliveParticles;
}


glm::vec4 regionCheck(const glm::vec4& v, const glm::vec4& origin)
{
    std::array<glm::vec4, 6> regionPoints = {
        glm::vec4(origin.x, origin.y + 1.0f, origin.z, 1.0f),
        glm::vec4(origin.x, origin.y - 1.0f, origin.z, 1.0f),

        glm::vec4(origin.x + 1.0f, origin.y, origin.z + 1.0f, 1.0f),
        glm::vec4(origin.x - 1.0f, origin.y, origin.z - 1.0f, 1.0f),
        glm::vec4(origin.x + 1.0f, origin.y, origin.z - 1.0f, 1.0f),
        glm::vec4(origin.x - 1.0f, origin.y, origin.z + 1.0f, 1.0f)
    };
    float distance;
    float angle;
    for (const auto& point : regionPoints)
    {
        distance = glm::distance(origin, point);
        angle = glm::asin(0.85 / distance);
        if (glm::dot(v, glm::normalize(point - origin)) > cos(angle))
        {
            return point;
        }
    }
    return origin;
}

void createFirework(std::vector<Particle>& particles, const glm::vec4& position, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position;
        p.alpha = glm::vec4(1.0f);
        float theta = glm::linearRand(0.0f, glm::two_pi<float>());
        float phi = glm::linearRand(0.0f, glm::pi<float>());
        float speed = glm::linearRand(5.0f, 10.0f);
        p.velocity.x = speed * sin(phi) * cos(theta);
        p.velocity.y = speed * sin(phi) * sin(theta);
        p.velocity.z = speed * cos(phi);
        p.velocity.w = 1.0f;
        p.lifetime = 3.0f;
        p.regionPoint = regionCheck(glm::normalize(p.velocity), position);
        if (p.regionPoint != position)
            p.swirl = 1.0f;
        else
            p.swirl = 0.0f;
        p.fadeRate = 1.0f / 3.0f;
        p.originY = position.y;
        p.origin = position;
        particles[i] = p;
    }
}

Particle defaultParticle()
{
    Particle p;
    p.position = glm::vec4(0.0f);
    p.velocity = glm::vec4(0.0f);
    p.alpha = glm::vec4(0.0f);
    p.regionPoint = glm::vec4(0.0f);
    p.lifetime = 0.0f; 
    p.swirl = 0.0f;
    p.fadeRate = 0.0f;
    p.originY = 0.0f;
    p.origin = glm::vec4(0.0f);
    return p;
}

void displayFPS(GLFWwindow* window) {
    static double previousSeconds = glfwGetTime();
    static int frameCount = 0;

    // Calculate time elapsed since last frame
    double currentSeconds = glfwGetTime();
    double elapsedSeconds = currentSeconds - previousSeconds;

    // Update FPS every second
    if (elapsedSeconds >= 1.0) {
        double fps = static_cast<double>(frameCount) / elapsedSeconds;
        // Print FPS to console
        std::cout << "FPS: " << fps << std::endl;

        // Reset frame count and timer
        frameCount = 0;
        previousSeconds = currentSeconds;
    }

    // Increment frame count
    frameCount++;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    /*if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOGlobal);
        Particle* particleData = (Particle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, lastUsedId,  particleNum* sizeof(Particle), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        for (GLuint i = lastUsedId; i < lastUsedId + particleNum; ++i)
        {
            Particle p;
            particleData[i].position = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
            particleData[i].alpha = glm::vec4(1.0f);
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(5.0f, 10.0f);
            particleData[i].velocity.x = speed * sin(phi) * cos(theta);
            particleData[i].velocity.y = speed * sin(phi) * sin(theta);
            particleData[i].velocity.z = speed * cos(phi);
            particleData[i].velocity.w = 1.0f;
            particleData[i].lifetime = 3.0f;
            particleData[i].regionPoint = regionCheck(glm::normalize(p.velocity), glm::vec4(0.0f, 5.0f, 1.0f, 1.0f));
            if (particleData[i].regionPoint != glm::vec4(0.0f, 5.0f, 1.0f, 1.0f))
                particleData[i].swirl = 1.0f;
            else
                particleData[i].swirl = 0.0f;
            particleData[i].fadeRate = 1.0f / 3.0f;
            particleData[i].originY = 5.0f;
            particleData[i].origin = glm::vec4(0.0f, 5.0f, 1.0f, 1.0f);
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }*/
}

// callback function to change the window dimensions whenever it changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// callback function to change camera based on mouse movements
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

