#include "ScalarGrid3.h"

glm::vec3<double> ScalarGrid3::gradientAtDataPoint(size_t i, size_t j, size_t k)
{
	double left = _data[(i > 0) ? i - 1 : i][j][k];
	double right = _data[(i + 1 < dataSize().x) ? i + 1 : i][j][k];
	double down = _data[i][(j > 0) ? j - 1 : j][k];
	double up = _data[i][(j + 1 < dataSize().y) ? j + 1 : j][k];
	double back = _data[i][j][(k > 0) ? k - 1 : k];
	double front = _data[i][j][(k + 1 < dataSize().z) ? k + 1 : k];

	return 0.5 * glm::vec3<double>(right - left, up - down, front - back) / gridSpacing();
}

// gradient at any given point, interpolates from nearby locations
/*glm::vec3<double> ScalarGrid3::gradient(const glm::vec3<double>& x)
{
	std::array<
}*/