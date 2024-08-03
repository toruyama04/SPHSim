#pragma once
#include <memory>

#include "Field.h"
#include "Grid.h"


class ScalarGrid3 : public ScalarField3, public Grid3
{
public:
	ScalarGrid3(GLuint bindingPoint);

	virtual ~ScalarGrid3();

	virtual Size3 dataSize() const = 0;

	virtual glm::vec3<double> dataOrigin() const = 0;

	// buffer data
	void resize(const Size3 resolution, const glm::vec3<double>& gridSpacing = glm::vec3<double>(1, 1, 1),
		const glm::vec3<double>& origin = glm::vec3<double>(), double initialValue = 0.0);

	// getters
	const double& operator()(size_t i, size_t j, size_t k) const;

	double& operator()(size_t i, size_t j, size_t k);

	double laplacianAtDataPoint(size_t i, size_t j, size_t k) const;

	glm::vec3<double> gradientAtDataPoint(size_t i, size_t j, size_t k) const;

	glm::vec3<double> gradient(const glm::vec3<double>& x) const;

	double laplacian(const glm::vec3& x) const override;

	double sample(const glm::vec3& x) const override;

	// forEachDataPointIndex

	// getters and setters for the gridSSBO

private:
	// buffer
	GLuint _data;
	// std::vector<std::vector<std::vector<double>>> _data;
};

typedef std::shared_ptr<ScalarGrid3> ScalarGrid3Ptr;

//! Abstract base class for 3-D scalar grid builder.
class ScalarGridBuilder3 {
public:
	//! Creates a builder.
	ScalarGridBuilder3();

	//! Default destructor.
	virtual ~ScalarGridBuilder3();

	//! Returns 3-D scalar grid with given parameters.
	virtual ScalarGrid3Ptr build(
		const Size3& resolution,
		const glm::vec3<double>& gridSpacing,
		const glm::vec3<double>& gridOrigin,
		double initialVal) const = 0;
};

//! Shared pointer for the ScalarGridBuilder3 type.
typedef std::shared_ptr<ScalarGridBuilder3> ScalarGridBuilder3Ptr;

