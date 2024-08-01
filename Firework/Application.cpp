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
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE); // Exit if GLFW initialization fails
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(screen_width, screen_height, title, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE); // Exit if window creation fails
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
        std::exit(EXIT_FAILURE); // Exit if GLAD initialization fails
    }

    const GLubyte* version = glGetString(GL_VERSION);
    if (version != nullptr) {
        std::cout << "OpenGL Version: " << version << std::endl;
    }
    else {
        std::cerr << "Failed to retrieve OpenGL version\n";
    }

    int frame_width, frame_height;
    glfwGetFramebufferSize(window, &frame_width, &frame_height);
    if (frame_width == 0 || frame_height == 0) {
        std::cerr << "Failed to get framebuffer size\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(EXIT_FAILURE); // Exit if framebuffer size retrieval fails
    }
    glViewport(0, 0, frame_width, frame_height);

    first_mouse = true;
    deltaTime = 0.0f;
    lastFrame = 0.0f;
    this->screen_height = screen_height;
    this->screen_width = screen_width;
    last_x = static_cast<double>(screen_width) / 2.0f;
    last_y = static_cast<double>(screen_height) / 2.0f;

    camera = new Camera(glm::vec3(0.0f, 0.0f, 15.0f));
    if (camera == nullptr) {
        std::cerr << "Failed to create camera\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        std::exit(EXIT_FAILURE); // Exit if camera creation fails
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window))
	{
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput();
        //glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        firework->update(deltaTime);

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100.0f);

        firework->render(view, projection);

        // add fireworks based on user input?
        // add floor class or something like that

        displayFPS();
        glfwSwapBuffers(window);
	}
}

void Application::addFirework(Firework* firework_in)
{
    this->firework = firework_in;
    if (firework == nullptr)
    {
        std::cerr << "Firework add unsuccessful\n";
    }
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
