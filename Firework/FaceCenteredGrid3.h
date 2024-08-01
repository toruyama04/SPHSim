#pragma once
#include "VectorGrid3.h"

class FaceCenteredGrid3 final : public VectorGrid3
{
public:
	FaceCenteredGrid3();

	virtual ~FaceCenteredGrid3();

	double& u(size_t i, size_t j, size_t k);

	const double& u(size_t i, size_t j, size_t k) const;

	double& v(size_t i, size_t j, size_t k);

	const double& v(size_t i, size_t j, size_t k) const;

	double& w(size_t i, size_t j, size_t k);

	const double& w(size_t i, size_t j, size_t k) const;

protected:
	void onResize(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& origin,
		const glm::vec3<double>& initialValue) override;

private:
	std::vector<std::vector<std::vector<double>>> _dataU;
	std::vector<std::vector<std::vector<double>>> _dataV;
	std::vector<std::vector<std::vector<double>>> _dataW;

	glm::vec3<double> _dataOriginU;
	glm::vec3<double> _dataOriginV;
	glm::vec3<double> _dataOriginW;
};

typedef std::shared_ptr<FaceCenteredGrid3> FaceCenteredGrid3Ptr;