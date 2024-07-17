#include "Application.h"
#include <glad/glad.h>
#include <iostream>

Application::Application(unsigned int screen_width, unsigned int screen_height, const char* title) : screen_width{screen_width}, screen_height{screen_height}
{
    first_mouse = true;
    deltaTime = 0.0f;
    lastFrame = 0.0f;
    this->title = title;
    last_x = static_cast<float>(screen_width) / 2.0f;
    last_y = static_cast<float>(screen_height) / 2.0f;
    window = initialise();

    glEnable(GL_BLEND | GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);

    camera = new Camera(glm::vec3(0.0f, 0.0f, 15.0f));
}

Application::~Application()
{
    delete camera;
    delete firework;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::addFirework(Firework& firework)
{
    this->firework = firework;
}

void Application::run()
{
	while (!glfwWindowShouldClose(window))
	{
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput();
        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        firework->update(deltaTime);

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100.0f);

        firework->render(view, projection);

        // add fireworks based on user input?
        // add floor class or something like that

        displayFPS();
        glfwSwapBuffers(window);
        glfwPollEvents();
	}
}

GLFWwindow* Application::initialise()
{
	if (!glfwInit())
    {
        std::cerr << "failed to initialise GLFW\n";
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
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
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialise GLAD\n";
        return nullptr;
    }
    return window;
}

void Application::displayFPS() {
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

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        glViewport(0, 0, width, height);
    }
}

void Application::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->mouseMove(xpos, ypos);
    }
}

void Application::mouseMove(double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos;

    last_x = xpos;
    last_y = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void Application::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
}
