#pragma once

class Vector3 final
{
public:
	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 Forward;
	static const Vector3 Backward;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Right;
	static const Vector3 Left;

	Vector3() = default;
	constexpr Vector3(float x, float y, float z);
	constexpr Vector3(const Vector3& Vector);

	// vector operations
	Vector3 Cross(const Vector3& A) const;
	float Dot(const Vector3& A) const;
	float Magnitude();
	void Normalize();
	Vector3 Perpendicular();

	// static vector ops
	static float Magnitude(const Vector3& A);
	static Vector3 Cross(const Vector3& A, const Vector3& B);
	static float Dot(const Vector3& A, const Vector3& B);
	static float LengthSq(const Vector3& A);
	static Vector3 Normalize(const Vector3& A);
	static Vector3 Perpendicular(const Vector3& A);


	// assignment
	constexpr Vector3& operator=(const Vector3& Vector);

	// member access
	float operator[](int iPos) const;
	float& operator[](int iPos);

	// comparison
	bool operator==(const Vector3& Vector) const;
	bool operator!=(const Vector3& Vector) const;

	// arithmetic operations
	Vector3 operator+ (const Vector3& Vector) const;
	Vector3 operator- (const Vector3& Vector) const;
	Vector3 operator* (const Vector3& Vector) const;
	Vector3 operator* (float fScalar) const;
	Vector3 operator/ (const Vector3& Vector) const;
	Vector3 operator/ (float fScalar) const;
	Vector3 operator- () const;

	// arithmetic updates
	Vector3& operator+= (const Vector3& Vector);
	Vector3& operator-= (const Vector3& Vector);
	Vector3& operator*= (const Vector3& Vector);
	Vector3& operator*= (float fScalar);
	Vector3& operator/= (const Vector3& Vector);
	Vector3& operator/= (float fScalar);

	float x;
	float y;
	float z;
};

inline constexpr Vector3::Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z)
{
}

inline constexpr Vector3::Vector3(const Vector3& Vector)
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
}

inline Vector3 Vector3::Cross(const Vector3& Vector) const
{
	Vector3 vRet;
	vRet.x = y * Vector.z - z * Vector.y;
	vRet.y = z * Vector.x - x * Vector.z;
	vRet.z = x * Vector.y - y * Vector.x;
	return(vRet);
}

inline float Vector3::Dot(const Vector3& Vector) const
{
	float ret = 0.0f;
	ret = x * Vector.x;
	ret += y * Vector.y;
	ret += z * Vector.z;
	return ret;
}

inline float Vector3::Magnitude()
{
	float fLength = 0.0f;

	fLength += x * x;
	fLength += y * y;
	fLength += z * z;

	return(sqrt(fLength));
}

inline void Vector3::Normalize()
{
	float Mag = Magnitude();
	if (0.0f == Mag) Mag = 0.0001f;

	float fInvMag = (1.0f / Mag);

	x *= fInvMag;
	y *= fInvMag;
	z *= fInvMag;
}

inline Vector3 Vector3::Perpendicular()
{
	float xAbs = fabs(x);
	float yAbs = fabs(y);
	float zAbs = fabs(z);
	float minVal = std::min(std::min(xAbs, yAbs), zAbs);

	if (xAbs == minVal)
		return Cross(Vector3(1.0f, 0.0f, 0.0f));
	else if (yAbs == minVal)
		return Cross(Vector3(0.0f, 1.0f, 0.0f));
	else
		return Cross(Vector3(0.0f, 0.0f, 1.0f));
}

inline float Vector3::Magnitude(const Vector3& A)
{
	return sqrt(Dot(A, A));
}

inline Vector3 Vector3::Cross(const Vector3& A, const Vector3& B)
{
	return A.Cross(B);
}

inline float Vector3::Dot(const Vector3& A, const Vector3& B)
{
	return A.Dot(B);
}

inline float Vector3::LengthSq(const Vector3& A)
{
	return Dot(A, A);
}

inline Vector3 Vector3::Normalize(const Vector3& A)
{
	Vector3 vec = A;
	vec.Normalize();
	return vec;
}

inline Vector3 Vector3::Perpendicular(const Vector3& A)
{
	Vector3 vec = A;
	return vec.Perpendicular();
}

inline constexpr Vector3& Vector3::operator=(const Vector3& Vector)
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;

	return(*this);
}

inline float Vector3::operator[](int iPos) const
{
	if (iPos == 0) return(x);
	if (iPos == 1) return(y);
	return(z);
}

inline float& Vector3::operator[](int iPos)
{
	if (iPos == 0) return(x);
	if (iPos == 1) return(y);
	return(z);
}

inline bool Vector3::operator==(const Vector3& Vector) const
{
	if ((x - Vector.x) * (x - Vector.x) > 0.01f)
		return false;
	if ((y - Vector.y) * (y - Vector.y) > 0.01f)
		return false;
	if ((z - Vector.z) * (z - Vector.z) > 0.01f)
		return false;

	return true;
}

inline bool Vector3::operator!=(const Vector3& Vector) const
{
	return(!(*this == Vector));
}

inline Vector3 Vector3::operator+(const Vector3& Vector) const
{
	Vector3 sum;

	sum.x = x + Vector.x;
	sum.y = y + Vector.y;
	sum.z = z + Vector.z;

	return(sum);
}

inline Vector3 Vector3::operator-(const Vector3& Vector) const
{
	Vector3 diff;

	diff.x = x - Vector.x;
	diff.y = y - Vector.y;
	diff.z = z - Vector.z;

	return(diff);
}

inline Vector3 Vector3::operator*(const Vector3& Vector) const
{
	Vector3 prod;

	prod.x = x * Vector.x;
	prod.y = y * Vector.y;
	prod.z = z * Vector.z;

	return(prod);
}

inline Vector3 Vector3::operator*(float fScalar) const
{
	Vector3 prod;

	prod.x = x * fScalar;
	prod.y = y * fScalar;
	prod.z = z * fScalar;

	return(prod);
}

inline Vector3 Vector3::operator/(const Vector3& Vector) const
{
	Vector3 quot;
	quot.x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
	quot.y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
	quot.z = Vector.z != 0.0f ? z / Vector.z : 0.0f;

	return(quot);
}

inline Vector3 Vector3::operator/(float fScalar) const
{
	Vector3 quot;
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		quot.x = x * fInvScalar;
		quot.y = y * fInvScalar;
		quot.z = z * fInvScalar;
	}
	else
	{
		quot = { 0.0f, 0.0f, 0.0f };
	}

	return(quot);
}

inline Vector3 Vector3::operator-() const
{
	Vector3 neg;

	neg.x = -x;
	neg.y = -y;
	neg.z = -z;

	return(neg);
}

inline Vector3& Vector3::operator+=(const Vector3& Vector)
{
	x += Vector.x;
	y += Vector.y;
	z += Vector.z;

	return(*this);
}

inline Vector3& Vector3::operator-=(const Vector3& Vector)
{
	x -= Vector.x;
	y -= Vector.y;
	z -= Vector.z;

	return(*this);
}

inline Vector3& Vector3::operator*=(const Vector3& Vector)
{
	x *= Vector.x;
	y *= Vector.y;
	z *= Vector.z;

	return(*this);
}

inline Vector3& Vector3::operator*=(float fScalar)
{
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;

	return(*this);
}

inline Vector3& Vector3::operator/=(const Vector3& Vector)
{
	x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
	y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
	z = Vector.z != 0.0f ? z / Vector.z : 0.0f;

	return(*this);
}

inline Vector3& Vector3::operator/=(float fScalar)
{
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		x *= fInvScalar;
		y *= fInvScalar;
		z *= fInvScalar;
	}
	else
	{
		x = y = z = 0.0f;
	}

	return(*this);
}
