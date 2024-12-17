#pragma once

#include "Vector3.h"

class Ray3 final
{
public:
	Ray3() = default;
	Ray3(const Vector3& position, const Vector3& direction);

	Vector3 origin;
	Vector3 direction;
};

inline Ray3::Ray3(const Vector3& Position, const Vector3& Direction)
{
	origin = Position;
	direction = Direction;
}