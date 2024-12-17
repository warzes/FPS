#pragma once

#include "Vector3.h"

class Plane3 final
{
public:
	Plane3() = default;
	Plane3(float A, float B, float C, float D);

	// Operations
	void Normalize();
	float DistanceToPoint(const Vector3& pt) const;

	float a = 0.0f;
	float b = 0.0f;
	float c = 0.0f;
	float d = 0.0f;
};

inline Plane3::Plane3(float A, float B, float C, float D)
{
	a = A;
	b = B;
	c = C;
	d = D;
}

inline void Plane3::Normalize()
{
	float fMagnitude = sqrt(a * a + b * b + c * c);

	a /= fMagnitude;
	b /= fMagnitude;
	c /= fMagnitude;
	d /= fMagnitude;
}

inline float Plane3::DistanceToPoint(const Vector3& pt) const
{
	return (a * pt.x + b * pt.y + c * pt.z + d);
}