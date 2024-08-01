#pragma once
#include <memory>

#include "Field.h"
#include "Grid.h"


class VectorGrid3 : public VectorField3, public Grid3
{
public:
	VectorGrid3();

	virtual ~VectorGrid3();

	void resize(const Size3 resolution, const glm::vec3<double>& gridSpacing = glm::vec3<double>(1, 1, 1),
		const glm::vec3<double>& origin = glm::vec3<double>(), const glm::vec3<double>& initialValue = glm::vec3<double>());

protected:
	virtual void onResize(const Size3& resolution, const glm::vec3<double>& gridSpacing, const glm::vec3<double>& origin,
		const glm::vec3<double>& intialValue) = 0;
};

typedef std::shared_ptr<VectorGrid3> VectorGrid3Ptr;

class VectorGridBuilder3 {
public:
    //! Creates a builder.
    VectorGridBuilder3();

    //! Default destructor.
    virtual ~VectorGridBuilder3();

    //! Returns 3-D vector grid with given parameters.
    virtual VectorGrid3Ptr build(
        const Size3& resolution,
        const glm::vec3<double>& gridSpacing,
        const glm::vec3<double>& gridOrigin,
        const glm::vec3<double>& initialVal) const = 0;
};

typedef std::shared_ptr<VectorGridBuilder3> VectorGridBuilder3Ptr;