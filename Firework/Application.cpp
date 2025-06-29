#include <glad/glad.h>

#include "Application.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void glfw_error_callback(int error, const char* desc) {
    std::cerr << "glfw error: " << desc << "\n";
}

void glfw_cursor_pos_callback(GLFWwindow* glfw_window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(glfw_window));
    if (app) {
        app->mouseMove(xpos, ypos);
    }
}

void Application::mouseMove(double xpos, double ypos) {
    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    double xoffset = xpos - last_x;
    double yoffset = last_y - ypos;

    last_x = xpos;
    last_y = ypos;

    camera->ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleFramebufferSize(width, height);
    }
}

void Application::handleFramebufferSize(int width, int height)
{
    glViewport(0, 0, width, height);
}


Application::Application(unsigned int screen_width, unsigned int screen_height, const char* title)
{
    // Initialisation: setting error/cursor/framebuffersize callback, OpenGL version
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(screen_width, screen_height, title, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load OpenGL functions\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    int frame_width, frame_height;
    glfwGetFramebufferSize(window, &frame_width, &frame_height);
    if (frame_width == 0 || frame_height == 0) {
        std::cerr << "Failed to get framebuffer size\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glViewport(0, 0, frame_width, frame_height);

    // Setting instance variables
    first_mouse = true;
    lastFrame = 0.0f;
    this->screen_height = screen_height;
    this->screen_width = screen_width;
    accumulator = 0.0f;
    last_x = static_cast<double>(screen_width) / 2.0f;
    last_y = static_cast<double>(screen_height) / 2.0f;

    // Creating camera (from learnopengl.com)
    camera = new Camera(glm::vec3(5.0f, 4.0f, 25.0f));
    if (camera == nullptr) {
        std::cerr << "Failed to create camera\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
}

Application::~Application()
{
    std::cout << "Deleting camera\n";
    delete camera;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::run()
{
    accumulator = 0.0f;
    lastFrame = 0.0f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window))
	{
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        float frameTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        accumulator += frameTime;

        processInput();
        glClearColor(0.75f, 0.75f, 0.75f, 1.0f);

        while (accumulator >= fixedTimeStep)
        {
            sim->update(fixedTimeStep);
            accumulator -= fixedTimeStep;
        }

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100.0f);

        sim->render(view, projection);

        // displayFPS();
        glfwSwapBuffers(window);
	}
}

void Application::addSim(Sim* sim_in)
{
    this->sim = sim_in;
    if (sim == nullptr)
    {
        std::cerr << "Sim add unsuccessful\n";
    }
}

/*void Application::displayFPS() {
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
}*/

void Application::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, 0.001f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, 0.001f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, 0.001f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, 0.001f);
}
