#pragma once

class Quaternion final
{
public:
	Quaternion() = default;
	Quaternion(float w, float x, float y, float z);

	float Length() const;
	float LengthSquared() const;
	float Dot(const Quaternion& a) const;

	Quaternion Conjugate() const;
	Quaternion Inverse() const;

	Quaternion operator+(const Quaternion& a) const;
	Quaternion operator-(const Quaternion& a) const;
	//Quaternion operator*(const Quaternion& a) const;
	//Quaternion operator/(const Quaternion& a) const;
	Quaternion operator*(float real) const;
	Quaternion operator/(float real) const;

	float w = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

Quaternion::Quaternion(float W, float X, float Y, float Z) : w(W), x(X), y(Y), z(Z)
{
}

inline float Quaternion::Length() const
{
	return(sqrt(w * w + x * x + y * y + z * z));
}

inline float Quaternion::LengthSquared() const
{
	return(w * w + x * x + y * y + z * z);
}

inline float Quaternion::Dot(const Quaternion& a) const
{
	return(w * a.w + x * a.x + y * a.y + z * a.z);
}

inline Quaternion Quaternion::Conjugate() const
{
	return{ w, -x, -y, -z };
}

inline Quaternion Quaternion::Inverse() const
{
	return(Conjugate() / LengthSquared());
}

inline Quaternion Quaternion::operator+(const Quaternion& a) const
{
	return{ a.w + w, a.x + x, a.y + y, a.z + z };
}

inline Quaternion Quaternion::operator-(const Quaternion& a) const
{
	return{ a.w - w, a.x - x, a.y - y, a.z - z };
}

//inline Quaternion Quaternion::operator*(const Quaternion& a) const
//{
//
//}

//inline Quaternion Quaternion::operator/(const Quaternion& a) const
//{
//	return(*this * a.Inverse());
//}

inline Quaternion Quaternion::operator*(float real) const
{
	return{ w * real, x * real, y * real, z * real };
}

inline Quaternion Quaternion::operator/(float real) const
{
	assert(real != 0.0);
	return{ w / real, x / real, y / real, z / real };
}
