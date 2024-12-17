#pragma once

#include "Plane3.h"
#include "Vector3.h"
#include "Sphere.h"
#include "Matrix4.h"

class Frustum final
{
public:
	Frustum();
	Frustum(const Matrix4& ViewProjection);

	void Update(const Matrix4& ViewProjection, bool bNormalize);
	bool Test(const Vector3& TestPoint) const;
	bool Test(const Sphere& TestSphere) const;

	bool Intersects(const Sphere& test) const;
	bool Envelops(const Sphere& test) const;

	std::array<Plane3, 6> planes;
};

inline Frustum::Frustum()
{
	for (int i = 0; i < 6; i++)
	{
		planes[i].a = 0.0f;
		planes[i].b = 0.0f;
		planes[i].c = 0.0f;
		planes[i].d = 0.0f;
	}
}

inline Frustum::Frustum(const Matrix4& ViewProjection)
{
	Update(ViewProjection, true);
}

inline void Frustum::Update(const Matrix4& ViewProj, bool bNormalize)
{
	// Left Plane
	planes[0].a = ViewProj(0, 3) + ViewProj(0, 0);
	planes[0].b = ViewProj(1, 3) + ViewProj(1, 0);
	planes[0].c = ViewProj(2, 3) + ViewProj(2, 0);
	planes[0].d = ViewProj(3, 3) + ViewProj(3, 0);

	// Right Plane
	planes[1].a = ViewProj(0, 3) - ViewProj(0, 0);
	planes[1].b = ViewProj(1, 3) - ViewProj(1, 0);
	planes[1].c = ViewProj(2, 3) - ViewProj(2, 0);
	planes[1].d = ViewProj(3, 3) - ViewProj(3, 0);

	// Top Plane
	planes[2].a = ViewProj(0, 3) - ViewProj(0, 1);
	planes[2].b = ViewProj(1, 3) - ViewProj(1, 1);
	planes[2].c = ViewProj(2, 3) - ViewProj(2, 1);
	planes[2].d = ViewProj(3, 3) - ViewProj(3, 1);

	// Bottom Plane
	planes[3].a = ViewProj(0, 3) + ViewProj(0, 1);
	planes[3].b = ViewProj(1, 3) + ViewProj(1, 1);
	planes[3].c = ViewProj(2, 3) + ViewProj(2, 1);
	planes[3].d = ViewProj(3, 3) + ViewProj(3, 1);

	// Near Plane
	planes[4].a = ViewProj(0, 2);
	planes[4].b = ViewProj(1, 2);
	planes[4].c = ViewProj(2, 2);
	planes[4].d = ViewProj(3, 2);

	// Far Plane
	planes[5].a = ViewProj(0, 3) - ViewProj(0, 2);
	planes[5].b = ViewProj(1, 3) - ViewProj(1, 2);
	planes[5].c = ViewProj(2, 3) - ViewProj(2, 2);
	planes[5].d = ViewProj(3, 3) - ViewProj(3, 2);


	// Normalize all planes
	if (bNormalize)
	{
		for (int i = 0; i < 6; i++)
		{
			planes[i].Normalize();
		}
	}
}

inline bool Frustum::Test(const Vector3& TestPoint) const
{
	// Test the point against each plane
	if (planes[0].DistanceToPoint(TestPoint) < 0)
		return(false);

	if (planes[1].DistanceToPoint(TestPoint) < 0)
		return(false);

	if (planes[2].DistanceToPoint(TestPoint) < 0)
		return(false);

	if (planes[3].DistanceToPoint(TestPoint) < 0)
		return(false);

	if (planes[4].DistanceToPoint(TestPoint) < 0)
		return(false);

	if (planes[5].DistanceToPoint(TestPoint) < 0)
		return(false);

	// If all tests passed, point is within the frustum
	return(true);
}

inline bool Frustum::Test(const Sphere& TestSphere) const
{
	// Test the center against each plane and compare the radius

	float fTemp = 0.0f;

	for (int i = 0; i < 6; i++)
	{
		fTemp = planes[i].DistanceToPoint(TestSphere.center);
		if (fTemp < -TestSphere.radius)
			return(false);

		if (float(fabs(fTemp)) < TestSphere.radius)
			return(true);
	}

	// If all tests passed, sphere is at least intersecting the frustum
	return(true);
}

inline bool Frustum::Intersects(const Sphere& bounds) const
{
	// distance to point plus the radius must be greater than zero to be intersecting!!!

	for (int i = 0; i < 6; i++)
	{
		if (planes[i].DistanceToPoint(bounds.center) + bounds.radius < 0)
			return(false);
	}

	// must not be enveloped if it is intersecting

	//if ( Envelops( bounds ) )
	//	return( false );

	// if all tests passed, then sphere is enveloped

	return(true);
}

inline bool Frustum::Envelops(const Sphere& bounds) const
{
	// distance to point minus the radius must be greater than zero to be enveloped!!!

	for (int i = 0; i < 6; i++)
	{
		if (planes[i].DistanceToPoint(bounds.center) - bounds.radius < 0)
			return(false);
	}

	// If all tests passed, sphere is enveloped

	return(true);
}