#pragma once
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "Shader.h"
#include "Field.h"

#include <unordered_map>
#include "FaceCenteredGrid3.h"
#include "ScalarGrid3.h"
#include "SemiLagrangian3.h"
#include "PointHashGridSearcher3.h"

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

	// grid stuff
	/*const FaceCenteredGrid3Ptr& velocity() const;
	glm::vec3<double> gridOrigin() const;
	glm::vec3<double> gridSpacing() const;
	Size3 resolution() const;
	// resizeGrid(const Size3& newSize, const glm:vec3<double>& newGridSpacing, const glm::vec3<double>& newGridOrigin);*/


	void addFirework(const glm::vec3& origin);

	GLuint getAliveCount();
	void resetAliveCount();

private:
	void initBuffers();
	void initShaders();
	void initSolvers();

	unsigned int _particle_num, _max_particles, _firework_num;
	float firework_lifetime;

	// grid(Smoke/Fluid)Solver3 constants
	/*double viscosity_coefficient = 0.0;
	glm::vec3<double> gravity = glm::vec3<double>(0.0, -9.8, 0.0);
	size_t _smokeDensityDataId;
	double _smokeDiffusionCoefficient = 0.0;
	double _buoyancySmokeDensityFactor = -0.0000625;
	double _smokeDecayFactor = 0.001;*/
	// gridSystemData3 constants
	/*const Size3 _resolution = Size3(50, 100, 50);
	const glm::vec3<double> _gridSpacing;
	const glm::vec3<double> _gridOrigin;
	FaceCenteredGrid3Ptr _velocity;
	size_t _velocityIdx; // ??
	std::vector<ScalarGrid3Ptr> _scalarDataList;
	std::vector<VectorGrid3Ptr> _vectorDataList;
	std::vector<ScalarGrid3Ptr> _advectableScalarDataList;
	std::vector<VectorGrid3Ptr> _advectableVectorDataList;*/

	// particleSystemData
	double _radius = 1e-3;
	double _drag_coefficient;
	// double _restitution_coefficient = 0.0;
	glm::vec3 _gravity = glm::vec3<double>(0.0, 2.0, 0.0);
	double _targetDensity = 800.0;
	double _targetSpacing = 0.1;
	double _kernelRadiusOverTargetSpacing = 1.8;
	double _kernelRadius;

	std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
	std::vector<std::unique_ptr<PointHashGridSearcher3>> _neighbour_grids;

	GLuint VBO, VAO, EBO;
	GLuint positionsSSBO, velocitiesSSBO, aliveFlagSSBO, forcesSSBO, ScalarData, VectorData;
	GLuint newPositionSSBO, newVelocitySSBO;
	GLuint atomicCounterBuffer;
	GLuint drawIndirectBuffer;
	GLuint particleTexture;
	DrawElementsIndirectCommand cmd;
};
