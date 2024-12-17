#pragma once

#include "Vector3.h"

class Sphere final
{
public:
	Sphere() = default;
	Sphere(const Vector3& Center, float Radius);

	bool Intersects(const Sphere& test) const;
	bool Envelops(const Sphere& test) const;

	void SamplePosition(Vector3& position, float theta, float phi) const;
	void SampleNormal(Vector3& normal, float theta, float phi) const;
	void SamplePositionAndNormal(Vector3& position, Vector3& normal, float theta, float phi) const;

	Vector3 center = { 0.0f, 0.0f, 0.0f };
	float   radius = 0.0f;
};

inline Sphere::Sphere(const Vector3& Center, float Radius)
	: center(Center)
	, radius(Radius)
{
}

inline bool Sphere::Intersects(const Sphere& test) const
{
	Vector3 Dist;
	Dist = (center - test.center);
	return(Dist.Magnitude() < (radius + test.radius));
}

inline bool Sphere::Envelops(const Sphere& test) const
{
	Vector3 Dist;
	Dist = (center - test.center);
	return(radius > test.radius + Dist.Magnitude());
}

inline void Sphere::SamplePosition(Vector3& position, float theta, float phi) const
{
	position.x = radius * sinf(phi) * cosf(theta) + center.x;
	position.y = radius * cosf(phi) + center.y;
	position.z = radius * sinf(phi) * sinf(theta) + center.z;
}

inline void Sphere::SampleNormal(Vector3& normal, float theta, float phi) const
{
	normal.x = radius * sinf(phi) * cosf(theta);
	normal.y = radius * cosf(phi);
	normal.z = radius * sinf(phi) * sinf(theta);
	normal = Vector3::Normalize(normal);
}

inline void Sphere::SamplePositionAndNormal(Vector3& position, Vector3& normal, float theta, float phi) const
{
	normal.x = radius * sinf(phi) * cosf(theta);
	normal.y = radius * cosf(phi);
	normal.z = radius * sinf(phi) * sinf(theta);

	position = normal + center;
	normal = Vector3::Normalize(normal);
}
