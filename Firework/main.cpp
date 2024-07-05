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
#include <windows.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
GLuint updateParticles(float totalTime, GLuint* SSBO, GLuint ACB);
void displayFPS(GLFWwindow* window);
glm::vec4 regionCheck(const glm::vec4& v, const glm::vec4& origin);
void setupSSBO(unsigned int buf, std::vector<glm::vec4>& type, int bindPoint, std::vector<glm::vec4>& defaultValues);
unsigned int setupFloor(float* floorVertices, size_t size);


// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float totaltime = 0.0f;
bool done1 = false;
bool done2 = false;

const unsigned int maxParticles = 80000;
const unsigned int particleNum = 8000;
const unsigned int trailNum = 2;
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

    glm::vec3 fireworkPos[2] = {
        glm::vec3(0.0f, 5.0f, 1.0f),
        glm::vec3(5.0f, 8.0f, -5.0f)
    };

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

    float fade = 1.0 / 3.0;
    for (int i = 0; i < fireworkNum; i++) {
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
    unsigned int VAO, VBO, EBO, ACB, SSBO[5], DIB;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(5, SSBO);
    glGenBuffers(1, &ACB);
    glGenBuffers(1, &DIB);
    
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ACB);
    GLuint initialCount = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &initialCount, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ACB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

    std::vector<glm::vec4> defaultValues(maxParticles, glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f));
    setupSSBO(SSBO[0], positions, 0, defaultValues);
    setupSSBO(SSBO[1], velocity, 1, defaultValues);
    setupSSBO(SSBO[2], colour, 2, defaultValues);
    setupSSBO(SSBO[3], regionPoint, 3, defaultValues);
    setupSSBO(SSBO[4], origin, 4, defaultValues);

    /*glm::vec4 debug[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[0]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 7999 * sizeof(glm::vec4), sizeof(glm::vec4) * 2, &debug);
    std::cout << "particle: " << debug[0].y << "\ndead: " << debug[1].y << std::endl;*/

    /* floor */
    unsigned int floorVAO = setupFloor(floorVertices, sizeof(float) * 12);

    // shaders
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag");
    Shader computeShader("shaders/particle.comp");

    Shader floorShader("shaders/floor.vert", "shaders/floor.frag");
    floorShader.use();
    floorShader.setVec3("colour", glm::vec3(0.5f, 0.5f, 0.5f));

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
        computeShader.setVec3("grav", glm::vec3(0.0f, -2.81f, 0.0));
        computeShader.setFloat("maxLife", 3.0f);
        computeShader.setFloat("dampingFactor", 13.0);
        computeShader.setFloat("minVelocity", 0.1);
        computeShader.setFloat("attractionStrength", 0.75f);
        computeShader.setFloat("spiralness", 1.4);
        computeShader.setFloat("spiralAttractionStrength", 0.4);
        computeShader.setFloat("acceleration", 25.0f);
        computeShader.setFloat("trailLife", 1.5f);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ACB);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float), &initialCount);
    	GLuint aliveCount = updateParticles(totaltime, SSBO, ACB);

    	particleShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        particleShader.setMat4("model", model);
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        cmd.instanceCount = aliveCount;
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
GLuint updateParticles(float totalTime, GLuint* SSBO, GLuint ACB) {

	glDispatchCompute((maxParticles + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ACB);
    GLuint* count = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
    GLuint aliveCount = count[0];
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[3]);
    glm::vec4* regionPoints = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[0]);
    glm::vec4* positions = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[1]);
    glm::vec4* velocity = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[2]);
    glm::vec4* colours = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[4]);
    glm::vec4* origin = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * maxParticles, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    float fadeRate = 1.0 / 1.5;
    float trailVelocityFactor = 0.5f;
	for (int i = 0; i < aliveCount; ++i)
    {
	    if (regionPoints[i].w == 1.0f)
	    {
            GLuint newIndex = aliveCount + i + 1;
            if (newIndex >= maxParticles)
            {
                std::cerr << "exceeded " << std::endl;
                break;
            }
            regionPoints[newIndex] = glm::vec4(regionPoints[i].x, regionPoints[i].y, regionPoints[i].z, 0.0f);
            positions[newIndex] = positions[i];
            velocity[newIndex] = glm::vec4(velocity[i].x * trailVelocityFactor, velocity[i].y * trailVelocityFactor, velocity[i].z * trailVelocityFactor, fadeRate);
            origin[newIndex] = glm::vec4(origin[i].x, origin[i].y, origin[i].z, 1.5f);
            colours[newIndex] = glm::vec4(1.5f, 1.0f, 1.0f, 1.0f);
	    }
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[3]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[0]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[1]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[2]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO[4]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
    return aliveCount;
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

/*void createFirework(std::vector<Particle>& particles, const glm::vec4& position, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position;
        p.alpha = glm::vec4(1.0f);
        float theta = glm::linearRand(0.0f, glm::two_pi<float>());
        float phi = glm::linearRand(0.0f, glm::pi<float>());
        float speed = glm::linearRand(1.0f, 3.0f);
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
}*/

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
