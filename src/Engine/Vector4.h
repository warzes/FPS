#pragma once

#include "Vector3.h"

class Vector4 final
{
public:
	Vector4() = default;
	Vector4(float x, float y, float z, float w);
	Vector4(const Vector3& vector, float w);
	Vector4(const Vector4& Vector);

	// vector operations
	float Dot(Vector4& vector);
	void Normalize();
	float Magnitude();

	// Operators
	Vector4& operator=(const Vector4& Vector);

	// member access
	float operator[](int iPos) const;
	float& operator[](int iPos);

	// comparison
	bool operator==(const Vector4& Vector) const;
	bool operator!=(const Vector4& Vector) const;

	// arithmetic operations
	Vector4 operator+(const Vector4& Vector) const;
	Vector4 operator-(const Vector4& Vector) const;
	Vector4 operator*(float fScalar) const;
	Vector4 operator*(const Vector4& Vector) const;
	Vector4 operator/(float fScalar) const;
	Vector4 operator/(const Vector4& Vector) const;
	Vector4 operator-() const;

	// arithmetic updates
	Vector4& operator+=(const Vector4& Vector);
	Vector4& operator-=(const Vector4& Vector);
	Vector4& operator*=(float fScalar);
	Vector4& operator*=(const Vector4& Vector);
	Vector4& operator/=(float fScalar);
	Vector4& operator/=(const Vector4& Vector);

	float x;
	float y;
	float z;
	float w;
};

inline Vector4::Vector4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W)
{
}

inline Vector4::Vector4(const Vector3& Vector, float W)
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
	w = W;
}

inline Vector4::Vector4(const Vector4& Vector)
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
	w = Vector.w;
}

inline float Vector4::Dot(Vector4& Vector)
{
	float ret = 0.0f;

	ret += x * Vector.x;
	ret += y * Vector.y;
	ret += z * Vector.z;
	ret += w * Vector.w;

	return ret;
}

inline void Vector4::Normalize()
{
	float fInvMag = (1.0f / Magnitude());

	x *= fInvMag;
	y *= fInvMag;
	z *= fInvMag;
	w *= fInvMag;
}

inline float Vector4::Magnitude()
{
	float fLength = 0.0f;

	fLength += x * x;
	fLength += y * y;
	fLength += z * z;
	fLength += w * w;

	return(sqrt(fLength));
}

inline Vector4& Vector4::operator=(const Vector4& Vector)
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
	w = Vector.w;

	return *this;
}

inline float Vector4::operator[](int iPos) const
{
	if (iPos == 0) return(x);
	if (iPos == 1) return(y);
	if (iPos == 2) return(z);
	return(w);
}

inline float& Vector4::operator[](int iPos)
{
	if (iPos == 0) return(x);
	if (iPos == 1) return(y);
	if (iPos == 2) return(z);
	return(w);
}

inline bool Vector4::operator==(const Vector4& Vector) const
{
	if ((x - Vector.x) * (x - Vector.x) > 0.01f)
		return false;
	if ((y - Vector.y) * (y - Vector.y) > 0.01f)
		return false;
	if ((z - Vector.z) * (z - Vector.z) > 0.01f)
		return false;
	if ((w - Vector.w) * (w - Vector.w) > 0.01f)
		return false;

	return true;
}

inline bool Vector4::operator!=(const Vector4& Vector) const
{
	return(!(*this == Vector));
}

inline Vector4 Vector4::operator+(const Vector4& Vector) const
{
	Vector4 sum;

	sum.x = x + Vector.x;
	sum.y = y + Vector.y;
	sum.z = z + Vector.z;
	sum.w = w + Vector.w;

	return(sum);
}

inline Vector4 Vector4::operator-(const Vector4& Vector) const
{
	Vector4 diff;

	diff.x = x - Vector.x;
	diff.y = y - Vector.y;
	diff.z = z - Vector.z;
	diff.w = w - Vector.w;

	return(diff);
}

inline Vector4 Vector4::operator*(float fScalar) const
{
	Vector4 prod;

	prod.x = x * fScalar;
	prod.y = y * fScalar;
	prod.z = z * fScalar;
	prod.w = w * fScalar;

	return(prod);
}

inline Vector4 Vector4::operator*(const Vector4& Vector) const
{
	Vector4 prod;

	prod.x = x * Vector.x;
	prod.y = y * Vector.y;
	prod.z = z * Vector.z;
	prod.w = w * Vector.w;

	return(prod);
}

inline Vector4 Vector4::operator/(float fScalar) const
{
	Vector4 quot;
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		quot.x = x * fInvScalar;
		quot.y = y * fInvScalar;
		quot.z = z * fInvScalar;
		quot.w = w * fInvScalar;
	}
	else
	{
		quot = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	return(quot);
}

inline Vector4 Vector4::operator/(const Vector4& Vector) const
{
	Vector4 quot;
	quot.x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
	quot.y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
	quot.z = Vector.z != 0.0f ? z / Vector.z : 0.0f;
	quot.w = Vector.w != 0.0f ? w / Vector.w : 0.0f;

	return(quot);
}

inline Vector4 Vector4::operator-() const
{
	Vector4 neg;

	neg.x = -x;
	neg.y = -y;
	neg.z = -z;
	neg.w = -w;

	return(neg);
}

inline Vector4& Vector4::operator+=(const Vector4& Vector)
{
	x += Vector.x;
	y += Vector.y;
	z += Vector.z;
	w += Vector.w;

	return(*this);
}

inline Vector4& Vector4::operator-=(const Vector4& Vector)
{
	x -= Vector.x;
	y -= Vector.y;
	z -= Vector.z;
	w -= Vector.w;

	return(*this);
}

inline Vector4& Vector4::operator*=(float fScalar)
{
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;
	w *= fScalar;

	return(*this);
}

inline Vector4& Vector4::operator*=(const Vector4& Vector)
{
	x *= Vector.x;
	y *= Vector.y;
	z *= Vector.z;
	w *= Vector.w;

	return(*this);
}

inline Vector4& Vector4::operator/=(float fScalar)
{
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		x *= fInvScalar;
		y *= fInvScalar;
		z *= fInvScalar;
		w *= fInvScalar;
	}
	else
	{
		x = y = z = w = 0.0f;
	}

	return(*this);
}

inline Vector4& Vector4::operator/=(const Vector4& Vector)
{
	x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
	y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
	z = Vector.z != 0.0f ? z / Vector.z : 0.0f;
	w = Vector.w != 0.0f ? w / Vector.w : 0.0f;

	return(*this);
}
