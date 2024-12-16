#pragma once

class Vector2 final
{
public:
	Vector2() = default;
	Vector2(float x, float y);
	Vector2(const Vector2& Vector);

	// vector operations
 	void Normalize();
	float Magnitude();

	// assignment
	Vector2& operator=(const Vector2& Vector);

	// accessors
	float operator[](int iPos) const;
	float& operator[](int iPos);

	// boolean comparison
	bool operator==(const Vector2& Vector) const;
	bool operator!=(const Vector2& Vector) const;

	// arithmetic operations
	Vector2 operator+(const Vector2& Vector) const;
	Vector2 operator-(const Vector2& Vector) const;
	Vector2 operator*(const Vector2& Vector) const;
	Vector2 operator*(float fScalar) const;
	Vector2 operator/(float fScalar) const;
	Vector2 operator-() const;

	// arithmetic updates
	Vector2& operator+=(const Vector2& Vector);
	Vector2& operator-=(const Vector2& Vector);
	Vector2& operator*=(float fScalar);
	Vector2& operator/=(float fScalar);

	float x;
	float y;
};

inline Vector2::Vector2(float X, float Y) : x(X), y(Y)
{
}

inline Vector2::Vector2(const Vector2& Vector)
{
	x = Vector.x;
	y = Vector.y;
}

inline void Vector2::Normalize()
{
	float fInvMag = (1.0f / Magnitude());
	x *= fInvMag;
	y *= fInvMag;
}

inline float Vector2::Magnitude()
{
	float fLength = 0.0f;
	fLength += x * x;
	fLength += y * y;
	return(sqrtf(fLength));
}

inline Vector2& Vector2::operator=(const Vector2& Vector)
{
	x = Vector.x;
	y = Vector.y;
	return(*this);
}

inline float Vector2::operator[](int iPos) const
{
	if (iPos == 0) return(x);
	return(y);
}

inline float& Vector2::operator[](int iPos)
{
	if (iPos == 0) return(x);
	return(y);
}

inline bool Vector2::operator==(const Vector2& Vector) const
{
	if ((x - Vector.x) * (x - Vector.x) > 0.01f)
		return false;
	if ((y - Vector.y) * (y - Vector.y) > 0.01f)
		return false;

	return(true);
}

inline bool Vector2::operator!=(const Vector2& Vector) const
{
	return(!(*this == Vector));
}

inline Vector2 Vector2::operator+(const Vector2& Vector) const
{
	Vector2 sum;
	sum.x = x + Vector.x;
	sum.y = y + Vector.y;
	return(sum);
}

inline Vector2 Vector2::operator-(const Vector2& Vector) const
{
	Vector2 diff;
	diff.x = x - Vector.x;
	diff.y = y - Vector.y;
	return(diff);
}

inline Vector2 Vector2::operator*(const Vector2& Vector) const
{
	Vector2 prod;
	prod.x = x * Vector.x;
	prod.y = y * Vector.y;
	return(prod);
}

inline Vector2 Vector2::operator*(float fScalar) const
{
	Vector2 prod;
	prod.x = x * fScalar;
	prod.y = y * fScalar;
	return(prod);
}

inline Vector2 Vector2::operator/(float fScalar) const
{
	Vector2 quot;
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		quot.x = x * fInvScalar;
		quot.y = y * fInvScalar;
	}
	else
	{
		quot = { 0.0f, 0.0f };
	}

	return(quot);
}

inline Vector2 Vector2::operator-() const
{
	Vector2 neg;
	neg.x = -x;
	neg.y = -y;
	return(neg);
}

inline Vector2& Vector2::operator+=(const Vector2& Vector)
{
	x += Vector.x;
	y += Vector.y;
	return(*this);
}

inline Vector2& Vector2::operator-=(const Vector2& Vector)
{
	x -= Vector.x;
	y -= Vector.y;
	return(*this);
}

inline Vector2& Vector2::operator*=(float fScalar)
{
	x *= fScalar;
	y *= fScalar;
	return(*this);
}

inline Vector2& Vector2::operator/=(float fScalar)
{
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		x *= fInvScalar;
		y *= fInvScalar;
	}
	else
	{
		x = y = 0.0f;
	}

	return(*this);
}