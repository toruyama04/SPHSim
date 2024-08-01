#pragma once
#include <glm/vec3.hpp>
#include <cmath>


class Field3
{
public:
	Field3();

	virtual ~Field3();
};

class ScalarField3 : public Field3
{
public:
	ScalarField3();

	virtual ~ScalarField3();

	virtual double sample(const glm::vec3& x) const = 0;

	virtual glm::vec3 gradient(const glm::vec3& x) const = 0;

	virtual double laplacian(const glm::vec3& x) const = 0;
};

class VectorField3 : public Field3
{
public:
	VectorField3();

	virtual ~VectorField3();

	virtual glm::vec3 sample(const glm::vec3& x) const = 0;

	virtual double divergence(const glm::vec3& x) const = 0;

	virtual glm::vec3 curl(const glm::vec3& x) const = 0;
};


/*
class MyCustomScalarField3 final : public ScalarField3 {
public:
	double sample(const glm::vec3& x) const override
	{
		return std::sin(x.x) * std::sin(x.y) * std::sin(x.z);
	}

	glm::vec3 gradient(const glm::vec3& x) const override
	{
		return glm::vec3(std::cos(x.x) * std::sin(x.y) * std::sin(x.z),
						 std::sin(x.x) * std::cos(x.y) * std::sin(x.z),
						 std::sin(x.x) * std::sin(x.y) * std::cos(x.z));
	}

	double laplacian(const glm::vec3& x) const override
	{
		return -std::sin(x.x) * std::sin(x.y) * std::sin(x.z)
			- std::sin(x.x) * std::sin(x.y) * std::sin(x.z)
			- std::sin(x.x) * std::sin(x.y) * std::sin(x.z);
	}
};

class MyCustomVectorField3 final : public VectorField3 {
public:
	glm::vec3 sample(const glm::vec3& x) const override {
		return glm::vec3(std::sin(x.x) * std::sin(y),
						 std::sin(x.y) * std::sin(z),
						 std::sin(x.z) * std::sin(x));
	}

	double divergence(const glm::vec3& x) const override
	{
		return std::cos(x.x) * std::sin(x.y)
			+ std::cos(x.y) * std::sin(x.z)
			+ std::cos(x.z) * std::sin(x.x);
	}

	glm::vec3 curl(const glm::vec3& x) const override
	{
		return glm::vec3(-std::sin(x.y) * std::cos(x.z),
			-std::sin(x.z) * std::cos(x.x),
			-std::sin(x.x) * std::cos(x.y));

	}
};
*/
