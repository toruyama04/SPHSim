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

void GLAPIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* user_param) {
    if (severity != GL_DEBUG_SEVERITY_HIGH) {
        return;
    }
    auto type_str = type == GL_DEBUG_TYPE_ERROR ? "[ERROR]"
        : type == GL_DEBUG_TYPE_PERFORMANCE ? "[PERFORMANCE]"
        : type == GL_DEBUG_TYPE_MARKER ? "[MARKER]"
        : type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "[DEPRECATED]"
        : type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "[UNDEFINED BEHAVIOR]" : "[OTHER]";
    auto severity_str = severity == GL_DEBUG_SEVERITY_HIGH ? "[HIGH]"
        : severity == GL_DEBUG_SEVERITY_MEDIUM ? "[MEDIUM]"
        : severity == GL_DEBUG_SEVERITY_LOW ? "[LOW]" : "[NOTIFICATION]";
    std::cerr << "OpenGL callback " << type_str << " " << severity_str << " " << message << std::endl;
    if (type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH) {
        // exit(-1);
    }
}


Application::Application(unsigned int screen_width, unsigned int screen_height, const char* title)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        std::cerr << "failed to initialise GLFW\n";
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    window = glfwCreateWindow(screen_width, screen_height, title, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load OpenGL functions\n";
        return;
    }

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ensure synchronous output
        glDebugMessageCallback(gl_message_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    int frame_width, frame_height;
    glfwGetFramebufferSize(window, &frame_width, &frame_height);
    glViewport(0, 0, frame_width, frame_height);
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;

    first_mouse = true;
    deltaTime = 0.0f;
    lastFrame = 0.0f;
    last_x = static_cast<double>(screen_width) / 2.0f;
    last_y = static_cast<double>(screen_height) / 2.0f;

    camera = new Camera(glm::vec3(0.0f, 0.0f, 15.0f));
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
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
	while (!glfwWindowShouldClose(window))
	{
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput();
        glClearColor(0.05f, 0.05f, 0.05f, 0.05f);

        /*firework->update(deltaTime);
        checkOpenGLErrors("firework->update");

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100.0f);

        firework->render(view, projection);
        checkOpenGLErrors("firework->render");*/

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
