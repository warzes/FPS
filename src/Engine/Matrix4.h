#pragma once

#include "Matrix3.h"

class Matrix4 final
{
public:
	Matrix4() = default;
	Matrix4(bool bZero);
	Matrix4(const Matrix4& Matrix);
	Matrix4(float fM11, float fM12, float fM13, float fM14,
		float fM21, float fM22, float fM23, float fM24,
		float fM31, float fM32, float fM33, float fM34,
		float fM41, float fM42, float fM43, float fM44);
	Matrix4 Inverse() const;

	void RotationX(float fRadians);
	void RotationY(float fRadians);
	void RotationZ(float fRadians);
	void Scale(float fScale);
	void Translate(float fX, float fY, float fZ);

	Vector3 GetBasisX() const;
	Vector3 GetBasisY() const;
	Vector3 GetBasisZ() const;

	Vector3 GetTranslation() const;
	Matrix3 GetRotation() const;

	void SetRotation(const Matrix3& Rot);
	void SetTranslation(const Vector3& Trans);

	static Matrix4 RotationMatrixXYZ(float fRadiansX, float fRadiansY, float fRadiansZ);
	static Matrix4 RotationMatrixX(float fRadians);
	static Matrix4 RotationMatrixY(float fRadians);
	static Matrix4 RotationMatrixZ(float fRadians);
	static Matrix4 ScaleMatrix(float fScale);
	static Matrix4 ScaleMatrix(const Vector3& scale);
	static Matrix4 ScaleMatrixXYZ(float fX, float fY, float fZ);
	static Matrix4 TranslationMatrix(float fX, float fY, float fZ);
	static Matrix4 LookAtLHMatrix(Vector3& eye, Vector3& at, Vector3& up);
	static Matrix4 PerspectiveFovLHMatrix(float fovy, float aspect, float zn, float zf);
	static Matrix4 OrthographicLHMatrix(float zn, float zf, float width, float height);

	void MakeZero();
	void MakeIdentity();
	void MakeTranspose();

	static Matrix4 Zero();
	static Matrix4 Identity();
	Matrix4 Transpose();

	// Operators
	Matrix4& operator=(const Matrix4& Matrix);

	// member access
	float operator()(int iRow, int iCol) const;
	float& operator()(int iRow, int iCol);
	float operator[](int iPos) const;
	float& operator[](int iPos);

	void SetRow(int iRow, const Vector4& Vector);
	void SetRow(int iRow, const Vector3& Vector);
	Vector4 GetRow(int iRow) const;
	void SetColumn(int iCol, const Vector4& Vector);
	Vector4 GetColumn(int iCol) const;

	// comparison
	bool operator==(const Matrix4& Matrix) const;
	bool operator!=(const Matrix4& Matrix) const;

	// arithmetic operations
	Matrix4 operator+(const Matrix4& Matrix) const;
	Matrix4 operator-(const Matrix4& Matrix) const;
	Matrix4 operator*(const Matrix4& Matrix) const;
	Matrix4 operator*(float fScalar) const;
	Matrix4 operator/(float fScalar) const;
	Matrix4 operator-() const;

	// arithmetic updates
	Matrix4& operator+=(const Matrix4& Matrix);
	Matrix4& operator-=(const Matrix4& Matrix);
	Matrix4& operator*=(const Matrix4& Matrix);
	Matrix4& operator*=(float fScalar);
	Matrix4& operator/=(float fScalar);

	// matrix - vector operations
	Vector4 operator*(const Vector4& V) const;  // M * v

	static const int m11 = 0;
	static const int m12 = 1;
	static const int m13 = 2;
	static const int m14 = 3;

	static const int m21 = 4;
	static const int m22 = 5;
	static const int m23 = 6;
	static const int m24 = 7;

	static const int m31 = 8;
	static const int m32 = 9;
	static const int m33 = 10;
	static const int m34 = 11;

	static const int m41 = 12;
	static const int m42 = 13;
	static const int m43 = 14;
	static const int m44 = 15;


private:
	float m_afEntry[4 * 4];

	static int I(int iRow, int iCol); // iRow*N + iCol
};

inline Matrix4::Matrix4(bool bZero)
{
	if (bZero)
		memset(m_afEntry, 0, 4 * 4 * sizeof(float));
}

inline Matrix4::Matrix4(const Matrix4& Matrix)
{
	memcpy(m_afEntry, (void*)&Matrix, 16 * sizeof(float));
}

inline Matrix4::Matrix4(float fM11, float fM12, float fM13, float fM14, float fM21, float fM22, float fM23, float fM24, float fM31, float fM32, float fM33, float fM34, float fM41, float fM42, float fM43, float fM44)
{
	m_afEntry[0] = fM11;
	m_afEntry[1] = fM12;
	m_afEntry[2] = fM13;
	m_afEntry[3] = fM14;

	m_afEntry[4] = fM21;
	m_afEntry[5] = fM22;
	m_afEntry[6] = fM23;
	m_afEntry[7] = fM24;

	m_afEntry[8] = fM31;
	m_afEntry[9] = fM32;
	m_afEntry[10] = fM33;
	m_afEntry[11] = fM34;

	m_afEntry[12] = fM41;
	m_afEntry[13] = fM42;
	m_afEntry[14] = fM43;
	m_afEntry[15] = fM44;
}

inline Matrix4 Matrix4::Inverse() const
{
	float fA0 = m_afEntry[0] * m_afEntry[5] - m_afEntry[1] * m_afEntry[4];
	float fA1 = m_afEntry[0] * m_afEntry[6] - m_afEntry[2] * m_afEntry[4];
	float fA2 = m_afEntry[0] * m_afEntry[7] - m_afEntry[3] * m_afEntry[4];
	float fA3 = m_afEntry[1] * m_afEntry[6] - m_afEntry[2] * m_afEntry[5];
	float fA4 = m_afEntry[1] * m_afEntry[7] - m_afEntry[3] * m_afEntry[5];
	float fA5 = m_afEntry[2] * m_afEntry[7] - m_afEntry[3] * m_afEntry[6];
	float fB0 = m_afEntry[8] * m_afEntry[13] - m_afEntry[9] * m_afEntry[12];
	float fB1 = m_afEntry[8] * m_afEntry[14] - m_afEntry[10] * m_afEntry[12];
	float fB2 = m_afEntry[8] * m_afEntry[15] - m_afEntry[11] * m_afEntry[12];
	float fB3 = m_afEntry[9] * m_afEntry[14] - m_afEntry[10] * m_afEntry[13];
	float fB4 = m_afEntry[9] * m_afEntry[15] - m_afEntry[11] * m_afEntry[13];
	float fB5 = m_afEntry[10] * m_afEntry[15] - m_afEntry[11] * m_afEntry[14];

	float fDet = fA0 * fB5 - fA1 * fB4 + fA2 * fB3 + fA3 * fB2 - fA4 * fB1 + fA5 * fB0;

	Matrix4 kInv;
	kInv(0, 0) = +m_afEntry[5] * fB5 - m_afEntry[6] * fB4 + m_afEntry[7] * fB3;
	kInv(1, 0) = -m_afEntry[4] * fB5 + m_afEntry[6] * fB2 - m_afEntry[7] * fB1;
	kInv(2, 0) = +m_afEntry[4] * fB4 - m_afEntry[5] * fB2 + m_afEntry[7] * fB0;
	kInv(3, 0) = -m_afEntry[4] * fB3 + m_afEntry[5] * fB1 - m_afEntry[6] * fB0;
	kInv(0, 1) = -m_afEntry[1] * fB5 + m_afEntry[2] * fB4 - m_afEntry[3] * fB3;
	kInv(1, 1) = +m_afEntry[0] * fB5 - m_afEntry[2] * fB2 + m_afEntry[3] * fB1;
	kInv(2, 1) = -m_afEntry[0] * fB4 + m_afEntry[1] * fB2 - m_afEntry[3] * fB0;
	kInv(3, 1) = +m_afEntry[0] * fB3 - m_afEntry[1] * fB1 + m_afEntry[2] * fB0;
	kInv(0, 2) = +m_afEntry[13] * fA5 - m_afEntry[14] * fA4 + m_afEntry[15] * fA3;
	kInv(1, 2) = -m_afEntry[12] * fA5 + m_afEntry[14] * fA2 - m_afEntry[15] * fA1;
	kInv(2, 2) = +m_afEntry[12] * fA4 - m_afEntry[13] * fA2 + m_afEntry[15] * fA0;
	kInv(3, 2) = -m_afEntry[12] * fA3 + m_afEntry[13] * fA1 - m_afEntry[14] * fA0;
	kInv(0, 3) = -m_afEntry[9] * fA5 + m_afEntry[10] * fA4 - m_afEntry[11] * fA3;
	kInv(1, 3) = +m_afEntry[8] * fA5 - m_afEntry[10] * fA2 + m_afEntry[11] * fA1;
	kInv(2, 3) = -m_afEntry[8] * fA4 + m_afEntry[9] * fA2 - m_afEntry[11] * fA0;
	kInv(3, 3) = +m_afEntry[8] * fA3 - m_afEntry[9] * fA1 + m_afEntry[10] * fA0;

	float fInvDet = ((float)1.0) / fDet;
	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
			kInv(iRow, iCol) *= fInvDet;
	}

	return(kInv);
}

inline void Matrix4::RotationX(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = 1.0f;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = 0.0f;
	m_afEntry[3] = 0.0f;

	m_afEntry[4] = 0.0f;
	m_afEntry[5] = fCos;
	m_afEntry[6] = fSin;
	m_afEntry[7] = 0.0f;

	m_afEntry[8] = 0.0f;
	m_afEntry[9] = -fSin;
	m_afEntry[10] = fCos;
	m_afEntry[11] = 0.0f;

	m_afEntry[12] = 0.0f;
	m_afEntry[13] = 0.0f;
	m_afEntry[14] = 0.0f;
	m_afEntry[15] = 1.0f;
}

inline void Matrix4::RotationY(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = fCos;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = -fSin;
	m_afEntry[3] = 0.0f;

	m_afEntry[4] = 0.0f;
	m_afEntry[5] = 1.0f;
	m_afEntry[6] = 0.0f;
	m_afEntry[7] = 0.0f;

	m_afEntry[8] = fSin;
	m_afEntry[9] = 0.0f;
	m_afEntry[10] = fCos;
	m_afEntry[11] = 0.0f;

	m_afEntry[12] = 0.0f;
	m_afEntry[13] = 0.0f;
	m_afEntry[14] = 0.0f;
	m_afEntry[15] = 1.0f;
}

inline void Matrix4::RotationZ(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = fCos;
	m_afEntry[1] = fSin;
	m_afEntry[2] = 0.0f;
	m_afEntry[3] = 0.0f;

	m_afEntry[4] = -fSin;
	m_afEntry[5] = fCos;
	m_afEntry[6] = 0.0f;
	m_afEntry[7] = 0.0f;

	m_afEntry[8] = 0.0f;
	m_afEntry[9] = 0.0f;
	m_afEntry[10] = 1.0f;
	m_afEntry[11] = 0.0f;

	m_afEntry[12] = 0.0f;
	m_afEntry[13] = 0.0f;
	m_afEntry[14] = 0.0f;
	m_afEntry[15] = 1.0f;
}

inline void Matrix4::Scale(float fScale)
{
	m_afEntry[0] = fScale;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = 0.0f;
	m_afEntry[3] = 0.0f;

	m_afEntry[4] = 0.0f;
	m_afEntry[5] = fScale;
	m_afEntry[6] = 0.0f;
	m_afEntry[7] = 0.0f;

	m_afEntry[8] = 0.0f;
	m_afEntry[9] = 0.0f;
	m_afEntry[10] = fScale;
	m_afEntry[11] = 0.0f;

	m_afEntry[12] = 0.0f;
	m_afEntry[13] = 0.0f;
	m_afEntry[14] = 0.0f;
	m_afEntry[15] = 1.0f;
}

inline void Matrix4::Translate(float fX, float fY, float fZ)
{
	m_afEntry[0] = 1.0f;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = 0.0f;
	m_afEntry[3] = 0.0f;

	m_afEntry[4] = 0.0f;
	m_afEntry[5] = 1.0f;
	m_afEntry[6] = 0.0f;
	m_afEntry[7] = 0.0f;

	m_afEntry[8] = 0.0f;
	m_afEntry[9] = 0.0f;
	m_afEntry[10] = 1.0f;
	m_afEntry[11] = 0.0f;

	m_afEntry[12] = fX;
	m_afEntry[13] = fY;
	m_afEntry[14] = fZ;
	m_afEntry[15] = 1.0f;
}

inline Vector3 Matrix4::GetBasisX() const
{
	Vector3 Basis;

	for (int i = 0; i < 3; i++)
		Basis[i] = m_afEntry[I(0, i)];

	return(Basis);
}

inline Vector3 Matrix4::GetBasisY() const
{
	Vector3 Basis;

	for (int i = 0; i < 3; i++)
		Basis[i] = m_afEntry[I(1, i)];

	return(Basis);
}

inline Vector3 Matrix4::GetBasisZ() const
{
	Vector3 Basis;

	for (int i = 0; i < 3; i++)
		Basis[i] = m_afEntry[I(2, i)];

	return(Basis);
}

inline Vector3 Matrix4::GetTranslation() const
{
	Vector3 Pos;

	for (int i = 0; i < 3; i++)
		Pos[i] = m_afEntry[I(3, i)];

	return(Pos);
}

inline Matrix3 Matrix4::GetRotation() const
{
	Matrix3 mRet;

	mRet.SetRow(0, GetBasisX());
	mRet.SetRow(1, GetBasisY());
	mRet.SetRow(2, GetBasisZ());

	return(mRet);
}

inline void Matrix4::SetRotation(const Matrix3& Rot)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m_afEntry[I(i, j)] = Rot[(3 * i + j)];
}

inline void Matrix4::SetTranslation(const Vector3& Trans)
{
	for (int i = 0; i < 3; i++)
		m_afEntry[I(3, i)] = Trans[i];
}

inline Matrix4 Matrix4::RotationMatrixXYZ(float fRadiansX, float fRadiansY, float fRadiansZ)
{
	return Matrix4::RotationMatrixZ(fRadiansZ) * Matrix4::RotationMatrixX(fRadiansX) * Matrix4::RotationMatrixY(fRadiansY);
}

inline Matrix4 Matrix4::RotationMatrixX(float fRadians)
{
	Matrix4 ret;
	ret.RotationX(fRadians);
	return(ret);
}

inline Matrix4 Matrix4::RotationMatrixY(float fRadians)
{
	Matrix4 ret;
	ret.RotationY(fRadians);
	return(ret);
}

inline Matrix4 Matrix4::RotationMatrixZ(float fRadians)
{
	Matrix4 ret;
	ret.RotationZ(fRadians);
	return(ret);
}

inline Matrix4 Matrix4::ScaleMatrix(float fScale)
{
	Matrix4 ret;
	ret.Scale(fScale);
	return(ret);
}

inline Matrix4 Matrix4::ScaleMatrix(const Vector3& scale)
{
	Matrix4 ret;

	ret.m_afEntry[0] = scale.x;
	ret.m_afEntry[1] = 0.0f;
	ret.m_afEntry[2] = 0.0f;
	ret.m_afEntry[3] = 0.0f;

	ret.m_afEntry[4] = 0.0f;
	ret.m_afEntry[5] = scale.y;
	ret.m_afEntry[6] = 0.0f;
	ret.m_afEntry[7] = 0.0f;

	ret.m_afEntry[8] = 0.0f;
	ret.m_afEntry[9] = 0.0f;
	ret.m_afEntry[10] = scale.z;
	ret.m_afEntry[11] = 0.0f;

	ret.m_afEntry[12] = 0.0f;
	ret.m_afEntry[13] = 0.0f;
	ret.m_afEntry[14] = 0.0f;
	ret.m_afEntry[15] = 1.0f;

	return(ret);
}

inline Matrix4 Matrix4::ScaleMatrixXYZ(float fX, float fY, float fZ)
{
	Matrix4 ret;

	ret.m_afEntry[0] = fX;
	ret.m_afEntry[1] = 0.0f;
	ret.m_afEntry[2] = 0.0f;
	ret.m_afEntry[3] = 0.0f;

	ret.m_afEntry[4] = 0.0f;
	ret.m_afEntry[5] = fY;
	ret.m_afEntry[6] = 0.0f;
	ret.m_afEntry[7] = 0.0f;

	ret.m_afEntry[8] = 0.0f;
	ret.m_afEntry[9] = 0.0f;
	ret.m_afEntry[10] = fZ;
	ret.m_afEntry[11] = 0.0f;

	ret.m_afEntry[12] = 0.0f;
	ret.m_afEntry[13] = 0.0f;
	ret.m_afEntry[14] = 0.0f;
	ret.m_afEntry[15] = 1.0f;

	return(ret);
}

inline Matrix4 Matrix4::TranslationMatrix(float fX, float fY, float fZ)
{
	Matrix4 ret;
	ret.Translate(fX, fY, fZ);
	return(ret);
}

inline Matrix4 Matrix4::LookAtLHMatrix(Vector3& eye, Vector3& at, Vector3& up)
{
	// This method is based on the method of the same name from the D3DX library.

	Matrix4 ret;

	Vector3 zaxis = at - eye;
	zaxis.Normalize();

	Vector3 xaxis = up.Cross(zaxis);
	xaxis.Normalize();

	Vector3 yaxis = zaxis.Cross(xaxis);

	ret.m_afEntry[0] = xaxis.x;
	ret.m_afEntry[1] = yaxis.x;
	ret.m_afEntry[2] = zaxis.x;
	ret.m_afEntry[3] = 0.0f;

	ret.m_afEntry[4] = xaxis.y;
	ret.m_afEntry[5] = yaxis.y;
	ret.m_afEntry[6] = zaxis.y;
	ret.m_afEntry[7] = 0.0f;

	ret.m_afEntry[8] = xaxis.z;
	ret.m_afEntry[9] = yaxis.z;
	ret.m_afEntry[10] = zaxis.z;
	ret.m_afEntry[11] = 0.0f;

	ret.m_afEntry[12] = -(xaxis.Dot(eye));
	ret.m_afEntry[13] = -(yaxis.Dot(eye));
	ret.m_afEntry[14] = -(zaxis.Dot(eye));
	ret.m_afEntry[15] = 1.0f;

	return(ret);
}

inline Matrix4 Matrix4::PerspectiveFovLHMatrix(float fovy, float aspect, float zn, float zf)
{
	// This method is based on the method of the same name from the D3DX library.

	Matrix4 ret;

	float tanY = tan(fovy / 2.0f);
	if (0.0f == tanY) tanY = 0.001f;
	float yScale = 1.0f / tanY;

	if (0.0f == aspect) aspect = 0.001f;
	float xScale = yScale / aspect;

	ret.m_afEntry[0] = xScale;
	ret.m_afEntry[1] = 0.0f;
	ret.m_afEntry[2] = 0.0f;
	ret.m_afEntry[3] = 0.0f;

	ret.m_afEntry[4] = 0.0f;
	ret.m_afEntry[5] = yScale;
	ret.m_afEntry[6] = 0.0f;
	ret.m_afEntry[7] = 0.0f;

	ret.m_afEntry[8] = 0.0f;
	ret.m_afEntry[9] = 0.0f;
	ret.m_afEntry[10] = zf / (zf - zn);
	ret.m_afEntry[11] = 1.0f;

	ret.m_afEntry[12] = 0.0f;
	ret.m_afEntry[13] = 0.0f;
	ret.m_afEntry[14] = -zn * zf / (zf - zn);
	ret.m_afEntry[15] = 0.0f;

	return(ret);
}

inline Matrix4 Matrix4::OrthographicLHMatrix(float zn, float zf, float width, float height)
{
	// This method is based on the method of the same name from the D3DX library.

	Matrix4 ret;


	if (zn == zf) zf = zn + 0.1f;
	if (width <= 0.0f) width = 1.0f;
	if (height <= 0.0f) height = 1.0f;

	ret.m_afEntry[0] = 2.0f / width;
	ret.m_afEntry[1] = 0.0f;
	ret.m_afEntry[2] = 0.0f;
	ret.m_afEntry[3] = 0.0f;

	ret.m_afEntry[4] = 0.0f;
	ret.m_afEntry[5] = 2.0f / height;
	ret.m_afEntry[6] = 0.0f;
	ret.m_afEntry[7] = 0.0f;

	ret.m_afEntry[8] = 0.0f;
	ret.m_afEntry[9] = 0.0f;
	ret.m_afEntry[10] = 1.0f / (zf - zn);
	ret.m_afEntry[11] = 0.0f;

	ret.m_afEntry[12] = 0.0f;
	ret.m_afEntry[13] = 0.0f;
	ret.m_afEntry[14] = zn / (zn - zf);
	ret.m_afEntry[15] = 1.0f;

	return(ret);
}

inline void Matrix4::MakeZero()
{
	memset(m_afEntry, 0, 4 * 4 * sizeof(float));
}

inline void Matrix4::MakeIdentity()
{
	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
		{
			if (iRow == iCol)
				m_afEntry[I(iRow, iCol)] = 1.0f;
			else
				m_afEntry[I(iRow, iCol)] = 0.0f;
		}
	}
}

inline void Matrix4::MakeTranspose()
{
	Matrix4 mTranspose;

	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
			mTranspose.m_afEntry[I(iRow, iCol)] = m_afEntry[I(iCol, iRow)];
	}

	memcpy(m_afEntry, mTranspose.m_afEntry, 4 * 4 * sizeof(float));
}

inline Matrix4 Matrix4::Zero()
{
	Matrix4 mReturn = Matrix4(true);

	return(mReturn);
}

inline Matrix4 Matrix4::Identity()
{
	Matrix4 mIdent;

	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
		{
			if (iRow == iCol)
				mIdent.m_afEntry[I(iRow, iCol)] = 1.0f;
			else
				mIdent.m_afEntry[I(iRow, iCol)] = 0.0f;
		}
	}

	return(mIdent);
}

inline Matrix4 Matrix4::Transpose()
{
	Matrix4 mTranspose;

	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
			mTranspose.m_afEntry[I(iRow, iCol)] = m_afEntry[I(iCol, iRow)];
	}

	return(mTranspose);
}

inline Matrix4& Matrix4::operator=(const Matrix4& Matrix)
{
	memcpy(m_afEntry, Matrix.m_afEntry, 16 * sizeof(float));
	return(*this);
}

inline float Matrix4::operator()(int iRow, int iCol) const
{
	return(m_afEntry[I(iRow, iCol)]);
}

inline float& Matrix4::operator()(int iRow, int iCol)
{
	return(m_afEntry[I(iRow, iCol)]);
}

inline float Matrix4::operator[](int iPos) const
{
	return(m_afEntry[iPos]);
}

inline float& Matrix4::operator[](int iPos)
{
	return(m_afEntry[iPos]);
}

inline void Matrix4::SetRow(int iRow, const Vector4& Vector)
{
	for (int iCol = 0; iCol < 4; iCol++)
		m_afEntry[I(iRow, iCol)] = Vector[iCol];
}

inline void Matrix4::SetRow(int iRow, const Vector3& Vector)
{
	for (int iCol = 0; iCol < 3; iCol++)
		m_afEntry[I(iRow, iCol)] = Vector[iCol];
}

inline Vector4 Matrix4::GetRow(int iRow) const
{
	Vector4 vRow;
	for (int iCol = 0; iCol < 4; iCol++)
		vRow[iCol] = m_afEntry[I(iRow, iCol)];

	return(vRow);
}

inline void Matrix4::SetColumn(int iCol, const Vector4& Vector)
{
	for (int iRow = 0; iRow < 4; iRow++)
		m_afEntry[I(iRow, iCol)] = Vector[iRow];
}

inline Vector4 Matrix4::GetColumn(int iCol) const
{
	Vector4 vCol;
	for (int iRow = 0; iRow < 4; iRow++)
		vCol[iRow] = m_afEntry[I(iRow, iCol)];

	return(vCol);
}

inline bool Matrix4::operator==(const Matrix4& Matrix) const
{
	return(memcmp(m_afEntry, Matrix.m_afEntry, 4 * 4 * sizeof(float)) == 0);
}

inline bool Matrix4::operator!=(const Matrix4& Matrix) const
{
	return(memcmp(m_afEntry, Matrix.m_afEntry, 4 * 4 * sizeof(float)) != 0);
}

inline Matrix4 Matrix4::operator+(const Matrix4& Matrix) const
{
	Matrix4 mSum;

	for (int i = 0; i < 4 * 4; i++)
		mSum.m_afEntry[i] = m_afEntry[i] + Matrix.m_afEntry[i];

	return(mSum);
}

inline Matrix4 Matrix4::operator-(const Matrix4& Matrix) const
{
	Matrix4 mDiff;

	for (int i = 0; i < 4 * 4; i++)
		mDiff.m_afEntry[i] = m_afEntry[i] - Matrix.m_afEntry[i];

	return(mDiff);
}

inline Matrix4 Matrix4::operator*(const Matrix4& Matrix) const
{
	Matrix4 mProd;

	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
		{
			int i = I(iRow, iCol);
			mProd.m_afEntry[i] = 0.0f;
			for (int iMid = 0; iMid < 4; iMid++)
			{
				mProd.m_afEntry[i] +=
					m_afEntry[I(iRow, iMid)] * Matrix.m_afEntry[I(iMid, iCol)];
			}
		}
	}
	return(mProd);
}

inline Matrix4 Matrix4::operator*(float fScalar) const
{
	Matrix4 mProd;

	for (int i = 0; i < 4 * 4; i++)
		mProd.m_afEntry[i] = m_afEntry[i] * fScalar;

	return(mProd);
}

inline Matrix4 Matrix4::operator/(float fScalar) const
{
	Matrix4 mQuot;
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		for (int i = 0; i < 4 * 4; i++)
			mQuot.m_afEntry[i] = m_afEntry[i] * fInvScalar;
	}
	else
	{
		for (int i = 0; i < 4 * 4; i++)
			mQuot.m_afEntry[i] = 0;
	}

	return(mQuot);
}

inline Matrix4 Matrix4::operator-() const
{
	Matrix4 mNeg;

	for (int i = 0; i < 4 * 4; i++)
		mNeg.m_afEntry[i] = -m_afEntry[i];

	return(mNeg);
}

inline Matrix4& Matrix4::operator+=(const Matrix4& Matrix)
{
	for (int i = 0; i < 4 * 4; i++)
		m_afEntry[i] += Matrix.m_afEntry[i];

	return(*this);
}

inline Matrix4& Matrix4::operator-=(const Matrix4& Matrix)
{
	for (int i = 0; i < 4 * 4; i++)
		m_afEntry[i] -= Matrix.m_afEntry[i];

	return(*this);
}

inline Matrix4& Matrix4::operator*=(const Matrix4& Matrix)
{
	Matrix4 mProd = *this;

	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
		{
			int i = I(iRow, iCol);
			m_afEntry[i] = 0.0f;
			for (int iMid = 0; iMid < 4; iMid++)
			{
				m_afEntry[i] +=
					mProd.m_afEntry[I(iRow, iMid)] * Matrix.m_afEntry[I(iMid, iCol)];
			}
		}
	}
	return(*this);
}

inline Matrix4& Matrix4::operator*=(float fScalar)
{
	for (int i = 0; i < 4 * 4; i++)
		m_afEntry[i] *= fScalar;

	return(*this);
}

inline Matrix4& Matrix4::operator/=(float fScalar)
{
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		for (int i = 0; i < 4 * 4; i++)
			m_afEntry[i] *= fInvScalar;
	}
	else
	{
		for (int i = 0; i < 4 * 4; i++)
			m_afEntry[i] = 0;
	}

	return(*this);
}

inline Vector4 Matrix4::operator*(const Vector4& Vector) const
{
	Vector4 vProd;
	for (int iCol = 0; iCol < 4; iCol++)
	{
		vProd[iCol] = 0.0f;
		for (int iRow = 0; iRow < 4; iRow++)
			vProd[iCol] += m_afEntry[I(iRow, iCol)] * Vector[iRow];
	}
	return(vProd);
}

inline int Matrix4::I(int iRow, int iCol)
{
	return(4 * iRow + iCol);
}


