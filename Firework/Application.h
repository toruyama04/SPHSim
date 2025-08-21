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

	void addSim(Sim* sim_in);

	void handleFramebufferSize(int width, int height);
	void mouseMove(float xpos, float ypos);
	void processInput(float dt);


private:
	Sim* sim = nullptr;
	GLFWwindow* window = nullptr;
	Camera* camera = nullptr;

	float screenWidth;
	float screenHeight;
	
	// CFL condition
	// weakly compressible -> C * (h / c + v_max) - could get Sim object to choose timestep
	float fixedTimeStep = 0.0005f;

	float lastFrame;
	float accumulator;
	float lastX;
	float lastY;
};
