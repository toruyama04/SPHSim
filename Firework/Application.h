#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "SPHSim.h"


class Application
{
public:
	Application() : Application(1920, 1080, "") {}
	Application(unsigned int screen_width, unsigned int screen_height, const char* title);
	~Application();

	void run();

	void displayFPS();
	void addSim(Sim* firework_in);

	void handleFramebufferSize(int width, int height);
	void mouseMove(double xpos, double ypos);
	void processInput();


private:
	Sim* sim = nullptr;
	float fixedTimeStep = 0.03f;
	float lastFrame;
	float accumulator;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	double last_x;
	double last_y;
	bool first_mouse;
	unsigned int screen_width;
	unsigned int screen_height;
};
