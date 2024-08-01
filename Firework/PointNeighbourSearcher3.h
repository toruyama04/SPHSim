#pragma once
#include <functional>
#include <glm/vec3.hpp>


struct Point3I
{
	size_t x;
	size_t y;
	size_t z;

	Point3I() : x(0), y(0), z(0) {};
	Point3I(size_t x, size_t y, size_t z) : x{ x }, y{ y }, z{ z } {};
};


class PointNeighborSearcher3
{
public:
	typedef std::function<void(size_t, const glm::vec3&)> ForEachNearbyPointFunc;

	PointNeighborSearcher3();

	virtual ~PointNeighborSearcher3();

	// 
	virtual void build(const ConstArrayAccessor1<glm::vec3>& points) = 0;

	// invokes the callback if there is a point near origin and within radius
	virtual void forEachNearbyPoint(const glm::vec3& origin, double radius, const ForEachNearbyPointFunc& callback) const = 0;

};

// for each bucket, we will store the indices of the points that fall into the bucket
// im assuming we want a PointHashGridSearcher for each firework, each holding points/buckets for the firework particles
class PointHashGridSearcher3 final : public PointNeighborSearcher3
{
public:
	PointHashGridSearcher3(const Size3& resolution, double gridSpacing);
	PointHashGridSearcher3(size_t resolutionX, size_t resolutionY, size_t resolutionZ, double gridSpacing);

	void build(const ConstArrayAccessor1<glm::vec3>& points) override;

	void forEachNearbyPoint(const glm::vec3& origin, double radius, const ForEachNearbyPointFunc& callback) const override;

	size_t getHashKeyFromPosition(const glm::vec3& position) const;

	size_t PointHashGridSearcher3::getHasKeyFromBucketIndex(const Point3I& bucketIndex) const;

	Point3I PointHashGridSearcher3::getBucketIndex(const glm::vec3& position) const;

private:
	double _gridSpacing = 1.0;
	Point3I _resolution = Point3I(1, 1, 1);
	std::vector<glm::vec3> _points;
	std::vector<std::vector<size_t>> _buckets;
};