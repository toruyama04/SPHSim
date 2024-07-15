#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader.h"

#include <string>
#include <map>


class Application
{
public:
	Application() : Application(1920, 1080) {}
	Application(unsigned int screen_width, unsigned int screen_height);
	virtual ~Application();

	virtual void run() = 0;

	GLFWwindow* initialise();
	void addShader(const std::string shader_name, const char* computePath);
	void addShader(const std::string shader_name, const char* vertexPath, const char* fragmentPath);
	void displayFPS();

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void framebufferSizeChanged(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void mouseMoved(double xpos, double ypos);
	void processInput(GLFWwindow* window);
	

protected:
	std::map<std::string, Shader> shaders;
	unsigned int screen_width;
	unsigned int screen_height;
	float deltaTime;
	float lastFrame;
	GLFWwindow* window;
	std::string title;
	Camera camera;
	double last_x;
	double last_y;
	bool first_mouse;
};
