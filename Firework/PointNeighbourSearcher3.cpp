#include "PointNeighbourSearcher3.h"


size_t PointHashGridSearcher3::getHasKeyFromBucketIndex(const Point3I& bucketIndex) const
{
	Point3I wrappedIndex = bucketIndex;
	wrappedIndex.x = bucketIndex.x % _resolution.x;
	wrappedIndex.y = bucketIndex.y % _resolution.y;
	wrappedIndex.z = bucketIndex.z % _resolution.z;
	if (wrappedIndex.x < 0) {
		wrappedIndex.x += _resolution.x;
	}
	if (wrappedIndex.y < 0) {
		wrappedIndex.y += _resolution.y;
	}
	if (wrappedIndex.z < 0) {
		wrappedIndex.z += _resolution.z;
	}
	return static_cast<size_t>((wrappedIndex.z * _resolution.y + wrappedIndex.y) * _resolution.x + wrappedIndex.x);
}

Point3I PointHashGridSearcher3::getBucketIndex(const glm::vec3& position) const
{
	Point3I bucketIndex;
	bucketIndex.x = static_cast<ssize_t>(std::floor(position.x / _gridSpacing));
	bucketIndex.y = static_cast<ssize_t>(std::floor(position.y / _gridSpacing));
	bucketIndex.z = static_cast<ssize_t>(std::floor(position.z / _gridSpacing));
	return bucketIndex;
}

size_t PointHashGridSearcher3::getHashKeyFromPosition(const glm::vec3& position) const
{
	Point3I bucketIndex = getBucketIndex(position);
	return getHasKeyFromBucketIndex(bucketIndex);
}


void PointHashGridSearcher3::build(const ConstArrayAccessor1<glm::vec3>& points)
{
	_buckets.clear();
	_points.clear();

	if (points.size() == 0)
		return;

	_buckets.resize(_resolution.x * _resolution.y * _resolution.z);
	_points.resize(points.size());

	for (size_t i = 0; i < points.size(); ++i)
	{
		_points[i] = points[i];
		size_t key = getHashKeyFromPosition(points[i]);
		_buckets[key].push_back(i);
	}
}
