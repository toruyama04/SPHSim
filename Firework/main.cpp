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
#include <time.h>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void updateParticles(std::vector<Particle>& particles, float deltaTime);
void createFirework(std::vector<Particle>& particles, const glm::vec3& position, int count);
void displayFPS(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 25.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int particleNum = 100000;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    float vertices[] = {
        -0.01f, -0.01f, 0.0f,
         0.01f, -0.01f, 0.0f,
        -0.01f,  0.01f, 0.0f,
         0.01f,  0.01f, 0.0f
    };
    float floorVertices[] = {
        // positions
        -10.0f, 0.0f, -10.0f,
         10.0f, 0.0f, -10.0f,
        -10.0f, 0.0f,  10.0f,
         10.0f, 0.0f,  10.0f
    };


    std::vector<Particle> particles;

    createFirework(particles, glm::vec3(0.0f, 10.0f, 0.0f), particleNum);

    unsigned int VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, particleNum * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);

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
    particleShader.use();
    glm::vec3 colour = glm::vec3(1.0f);
    particleShader.setVec3("vColor", colour);

    Shader floorShader("shaders/floor.vert", "shaders/floor.frag");
    floorShader.use();
    colour = glm::vec3(0.5f, 0.5f, 0.5f);
    floorShader.setVec3("colour", colour);
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        updateParticles(particles, deltaTime);

        std::vector<glm::vec3> positions(particles.size());
        for (size_t i = 0; i < particles.size(); ++i) {
            positions[i] = particles[i].position;
        }
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), &positions[0], GL_DYNAMIC_DRAW);

        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);

        particleShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        particleShader.setMat4("model", model);
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particles.size());
        glBindVertexArray(0);

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
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// function to update particles
void updateParticles(std::vector<Particle>& particles, float deltaTime) {
    for (auto& particle : particles) {
        if (particle.lifetime <= 1.0f) {
            particle.velocity += glm::vec3(0.0f, -0.03f, 0.0f);
        }
        particle.position += particle.velocity * deltaTime;
        particle.lifetime -= deltaTime;
        if (particle.lifetime <= 0.0f) {
            particle.position = glm::vec3(0.0f, 10.0f, 0.0f);
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
            float speed = glm::linearRand(1.0f, 1.3f);
            particle.velocity.x = speed * sin(phi) * cos(theta);
            particle.velocity.y = speed * sin(phi) * sin(theta);
            particle.velocity.z = speed * cos(phi);
            particle.lifetime = glm::linearRand(3.0f, 4.0f);
        }
    }
    /*particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.lifetime <= 0.0f; }), particles.end());*/
}


void createFirework(std::vector<Particle>& particles, const glm::vec3& position, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position;
        float theta = glm::linearRand(0.0f, glm::two_pi<float>());
        float phi = glm::linearRand(0.0f, glm::pi<float>());
        float speed = glm::linearRand(1.0f, 1.3f);
        p.velocity.x = speed * sin(phi) * cos(theta);
        p.velocity.y = speed * sin(phi) * sin(theta);
        p.velocity.z = speed * cos(phi);
        p.lifetime = glm::linearRand(3.0f, 4.0f);
        particles.push_back(p);
    }
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

