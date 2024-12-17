#pragma once

#include "Vector3.h"

class Box3 final
{
public:
	Box3() = default;
	Box3(const Vector3& Center, const Vector3& Forward, const Vector3& Up, const Vector3& Right, float fextents, float uextents, float rextents);

	Vector3				   center;
	std::array<Vector3, 3> axes;
	std::array<float, 3>   extents;
};

inline Box3::Box3(const Vector3& Center, const Vector3& Forward, const Vector3& Up, const Vector3& Right, float fextents, float uextents, float rextents)
{
	center = Center;

	axes[0] = Forward;
	axes[1] = Up;
	axes[2] = Right;

	extents[0] = fextents;
	extents[1] = uextents;
	extents[2] = rextents;
}