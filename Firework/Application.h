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

	// void displayFPS();
	void addSim(Sim* sim_in);

	void handleFramebufferSize(int width, int height);
	void mouseMove(double xpos, double ypos);
	void processInput();


private:
	Sim* sim = nullptr;

	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;
	unsigned int screen_width;
	unsigned int screen_height;
	
	// weakly compressible -> C * (h / c + v_max) - could get Sim object to choose timestep
	float fixedTimeStep = 0.0008f;
	float lastFrame;
	float accumulator;
	double last_x;
	double last_y;
	bool first_mouse;
};
