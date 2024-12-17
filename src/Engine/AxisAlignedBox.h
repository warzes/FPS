#pragma once

#include "Vector3.h"

class AxisAlignedBox final
{
public:
	AxisAlignedBox() = default;
	AxisAlignedBox(const Vector3& mins, const Vector3& maxs);

	bool Contains(const Vector3& p);
	void ExpandToPoint(const Vector3& p);

	Vector3 minimum = { 0.0f, 0.0f, 0.0f };
	Vector3 maximum = { 0.0f, 0.0f, 0.0f };
};

inline AxisAlignedBox::AxisAlignedBox(const Vector3& mins, const Vector3& maxs) :
	minimum(mins),
	maximum(maxs)
{
}

inline bool AxisAlignedBox::Contains(const Vector3& p)
{
	if (minimum.x < p.x && p.x < maximum.x &&
		minimum.y < p.y && p.y < maximum.y &&
		minimum.z < p.z && p.z < maximum.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

inline void AxisAlignedBox::ExpandToPoint(const Vector3& p)
{
	if (p.x > maximum.x) maximum.x = p.x;
	if (p.y > maximum.y) maximum.y = p.y;
	if (p.z > maximum.z) maximum.z = p.z;

	if (p.x < minimum.x) minimum.x = p.x;
	if (p.y < minimum.y) minimum.y = p.y;
	if (p.z < minimum.z) minimum.z = p.z;
}