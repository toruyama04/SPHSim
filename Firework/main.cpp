#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "shader.h"
#include "camera.h"
#include "utils.h"

#include <vector>
#include <random>
#include <iostream>
#include <array>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void updateParticles(Shader* compShader, GLuint* SSBO, GLuint ACB, GLuint flags);
void displayFPS(GLFWwindow* window);
glm::vec4 regionCheck(const glm::vec4& v, const glm::vec4& origin);
void setupSSBO(unsigned int buf, std::vector<glm::vec4>& type, int bindPoint, std::vector<glm::vec4>& defaultValues);
unsigned int setupFloor(float* floorVertices, size_t size);
GLFWwindow* initialiseOpenGL();


// settings
constexpr unsigned int SCR_WIDTH = 1920;
constexpr unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float lastX = static_cast<float>(SCR_WIDTH) / 2.0f;
float lastY = static_cast<float>(SCR_HEIGHT) / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

constexpr unsigned int maxParticles = 65536;
constexpr unsigned int particleNum = 4096;
constexpr unsigned int fireworkNum = 1;

int main() {
    GLFWwindow* window = initialiseOpenGL();
    // global states
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /*
     * position's w component is whether it swirls (whether it swirls to region-point)
     * velocity's w component is the fade-rate
     * origin's w component is its lifetime
     * regionPoint's w component is whether its a trail, 1 meaning it isn't
     * colour's x value is the maxlife
    */
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> velocity;
    std::vector<glm::vec4> colour;
    std::vector<glm::vec4> regionPoint;
    std::vector<glm::vec4> origin;

    float fade = 1.0f / 3.0f;
    for (unsigned int i = 0; i < fireworkNum; i++) {
        for (unsigned int j = 0; j < particleNum; ++j) {
            float theta = glm::linearRand(0.0f, glm::two_pi<float>());
            float phi = glm::linearRand(0.0f, glm::pi<float>());
			float speed = glm::linearRand(0.0f, 1.5f);
            velocity.emplace_back(speed * sin(phi) * cos(theta), speed * sin(phi) * sin(theta), speed * cos(phi), fade);
            colour.emplace_back(3.0f, 1.0f, 1.0f, 1.0f);
            regionPoint.push_back(regionCheck(glm::normalize(velocity[j]), glm::vec4(fireworkPos[i], 1.0f)));
            if (regionPoint[j] != glm::vec4(fireworkPos[i], 1.0f))
                positions.emplace_back(fireworkPos[i].x, fireworkPos[i].y, fireworkPos[i].z, 1.0f);
            else
                positions.emplace_back(fireworkPos[i].x, fireworkPos[i].y, fireworkPos[i].z, 0.0f);
            origin.emplace_back(fireworkPos[i], 3.0f);
        }
    }

    /* firework particles */
    unsigned int VAO, VBO, EBO, SSBO[5], DIB, ACB, flags;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &ACB);
    glGenBuffers(5, SSBO);
    glGenBuffers(1, &DIB);
    glGenBuffers(1, &flags);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, DIB);

    struct DrawElementsIndirectCommand
    {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLint baseVertex;
        GLuint baseInstance;
    };

    DrawElementsIndirectCommand cmd = { 6, 1, 0, 0, 0 };
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmd), &cmd, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
    GLuint initial = 0;
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initial, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, ACB);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    std::vector<glm::vec4> defaultValues(maxParticles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    setupSSBO(SSBO[0], positions, 0, defaultValues);
    setupSSBO(SSBO[1], velocity, 1, defaultValues);
    setupSSBO(SSBO[3], regionPoint, 3, defaultValues);
    setupSSBO(SSBO[4], origin, 4, defaultValues);
    setupSSBO(SSBO[2], colour, 2, defaultValues);

    std::vector<int> flagDefault(maxParticles, 0);
    std::vector<int> flagfirework(particleNum, 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flags);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * maxParticles, flagDefault.data(), GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * particleNum, flagfirework.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, flags);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /* floor */
    unsigned int floorVAO = setupFloor(floorVertices, sizeof(float) * 12);

    // shaders
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag");
    Shader computeShaderUpdate("shaders/particleUpdate.comp");
    Shader computeShaderPrefix("shaders/particlePrefix.comp");
    Shader computeShaderTrail("shaders/particleTrail.comp");
    Shader computeShaderReorder("shaders/particleReorder.comp");
    Shader compShaders[4] = { computeShaderUpdate, computeShaderTrail, computeShaderPrefix, computeShaderReorder };


    Shader floorShader("shaders/floor.vert", "shaders/floor.frag");
    floorShader.use();
    floorShader.setVec3("colour", glm::vec3(0.5f, 0.5f, 0.5f));

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	updateParticles(compShaders, SSBO, ACB, flags);

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
        GLuint* num = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        // std::cout << "alive: " << num[0] << "\n";

    	particleShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        particleShader.setMat4("model", model);
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        cmd.instanceCount = num[0];

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, DIB);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), &cmd, GL_DYNAMIC_DRAW);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);

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
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// function to update particles
void updateParticles(Shader* compShader, GLuint* SSBO, GLuint ACB, GLuint flags) {
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ACB);
    GLuint initial = 0;
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &initial);

    // update particle data
    compShader[0].use();
    compShader[0].setFloat("deltaTime", deltaTime);
    compShader[0].setVec3("grav", glm::vec3(0.0f, -2.81f, 0.0));
    compShader[0].setFloat("dampingFactor", 13.0f);
    compShader[0].setFloat("minVelocity", 0.1f);
    compShader[0].setFloat("attractionStrength", 0.95f);
    compShader[0].setFloat("spiralness", 1.4f);
    compShader[0].setFloat("spiralAttractionStrength", 0.7f);
    compShader[0].setFloat("acceleration", 25.0f);
	glDispatchCompute(maxParticles / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // prefix sum
    compShader[2].use();
    compShader[2].setInt("maxParticle", maxParticles);
    glDispatchCompute(maxParticles / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // reorder
    compShader[3].use();
    glDispatchCompute(maxParticles / 1024, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// add trail particles
    compShader[1].use();
    compShader[1].setFloat("trailFadeRate", 1.0f / 2.5f);
    compShader[1].setFloat("trailRate", 0.7f);
    compShader[1].setUInt("maxParticle", maxParticles);
    glDispatchCompute(particleNum / 256, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);  
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
    for (const auto& point : regionPoints)
    {
        float distance = glm::distance(origin, point);
        float angle = glm::asin(0.85f / distance);
        if (glm::dot(v, glm::normalize(point - origin)) > cos(angle))
        {
            return point;
        }
    }
    return origin;
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
        std::cout << "FPS: " << fps << "\n";

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

void setupSSBO(unsigned int buf, std::vector<glm::vec4>& type, int bindPoint, std::vector<glm::vec4>& defaultValues)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * maxParticles, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, defaultValues.data());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * particleNum, type.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

unsigned int setupFloor(float* floorVertices, size_t size)
{
    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, size, floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return floorVAO;
}

GLFWwindow* initialiseOpenGL()
{
    if (!glfwInit())
    {
        std::cerr << "failed to initialise GLFW\n";
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Firework", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return nullptr;
    }
    return window;
}
