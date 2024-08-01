#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"
#include "Field.h"

#include <unordered_map>
#include "FaceCenteredGrid3.h"
#include "ScalarGrid3.h"

class Firework
{
public:
	struct DrawElementsIndirectCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLint baseVertex;
		GLuint baseInstance;
	};

	Firework();
	~Firework();

	void render(const glm::mat4& view, const glm::mat4& projection);


	void update(float delta_time);
	void onBeginAdvanceTimeStep(double delta_time);
	void onEndAdvanceTimeStep(double delta_time);
	void computeGravity(double delta_time);
	void computeAdvection(double delta_time);
	void computeViscosity(double delta_time);
	void computeExternalForces(double delta_time);
	void computeDiffusion(double delta_time);
	void computeBuoyancyForce(double delta_time); // ?
	// ScalarField3Ptr fluidSDF() const;

	const FaceCenteredGrid3Ptr& velocity() const;
	glm::vec3<double> gridOrigin() const;
	glm::vec3<double> gridSpacing() const;
	Size3 resolution() const;
	// resizeGrid(const Size3& newSize, const glm:vec3<double>& newGridSpacing, const glm::vec3<double>& newGridOrigin);


	void addFirework(const glm::vec3& origin);

	GLuint getAliveCount();
	void resetAliveCount();

private:
	void initBuffers();
	void initShaders();
	void initSolvers();

	// updating functions
	void beginAdvanceTimeStep(double delta_time);
	void endAdvanceTimeStep(double delta_time);

	unsigned int _particle_num, _max_particles, _firework_num;
	float firework_lifetime;

	// grid(Smoke/Fluid)Solver3 constants
	double viscosity_coefficient = 0.0;
	glm::vec3<double> gravity = glm::vec3<double>(0.0, -9.8, 0.0);
	size_t _smokeDensityDataId;
	double _smokeDiffusionCoefficient = 0.0;
	double _buoyancySmokeDensityFactor = -0.0000625;
	double _smokeDecayFactor = 0.001;
	// temperature
	// _maxCfl
	

	// gridSystemData3 constants
	const Size3 _resolution = Size3(50, 100, 50);
	const glm::vec3<double> _gridSpacing;
	const glm::vec3<double> _origin;
	FaceCenteredGrid3Ptr _velocity;
	size_t _velocityIdx; // ??
	std::vector<ScalarGrid3Ptr> _scalarDataList;
	std::vector<VectorGrid3Ptr> _vectorDataList;
	std::vector<ScalarGrid3Ptr> _advectableScalarDataList;
	std::vector<VectorGrid3Ptr> _advectableVectorDataList;

	// smoke, temperature, buoyancy stuff

	std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;

	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, forcesSSBO, gridSSBO, neighboursSSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	GLuint particleTexture;
	DrawElementsIndirectCommand cmd;
};
