#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "Firework.h"


class Application
{
public:
	Application() : Application(1920, 1080, "") {}
	Application(unsigned int screen_width, unsigned int screen_height, const char* title);
	~Application();

	void run();

	GLFWwindow* initialise();
	void displayFPS();
	void addFirework(Firework* firework_in);
	void checkOpenGLErrors(const std::string& context);

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void handleFramebufferSize(int width, int height);
	void mouseMove(double xpos, double ypos);
	void processInput();


private:
	unsigned int screen_width;
	unsigned int screen_height;
	Firework* firework;

	float deltaTime;
	float lastFrame;
	GLFWwindow* window;
	const char* title;
	Camera* camera;
	double last_x;
	double last_y;
	bool first_mouse;
};
