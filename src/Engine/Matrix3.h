#pragma once

#include "Vector3.h"

class Matrix3 final
{
public:
	Matrix3() = default;
	Matrix3(bool bZero);
	Matrix3(const Matrix3& Matrix);
	Matrix3(
		float fM11, float fM12, float fM13,
		float fM21, float fM22, float fM23,
		float fM31, float fM32, float fM33);

	void RotationX(float fRadians);
	void RotationY(float fRadians);
	void RotationZ(float fRadians);
	void Rotation(Vector3& Rot);
	void RotationZYX(Vector3& Rot);
	void RotationEuler(Vector3& Axis, float Angle);
	void Orthonormalize();

	void MakeZero();
	void MakeIdentity();
	void MakeTranspose();

	Matrix3 Zero();
	Matrix3 Identity();
	Matrix3 Transpose();

	// Operators
	Matrix3& operator=(const Matrix3& Matrix);

	// member access
	float operator()(int iRow, int iCol) const;
	float& operator()(int iRow, int iCol);
	float operator[](int iPos) const;
	float& operator[](int iPos);

	void SetRow(int iRow, const Vector3& Vector);
	Vector3 GetRow(int iRow) const;
	void SetColumn(int iCol, const Vector3& Vector);
	Vector3 GetColumn(int iCol) const;

	// comparison
	bool operator==(const Matrix3& Matrix) const;
	bool operator!=(const Matrix3& Matrix) const;

	// arithmetic operations
	Matrix3 operator+(const Matrix3& Matrix) const;
	Matrix3 operator-(const Matrix3& Matrix) const;
	Matrix3 operator*(const Matrix3& Matrix) const;
	Matrix3 operator*(float fScalar) const;
	Matrix3 operator/(float fScalar) const;
	Matrix3 operator-() const;

	// arithmetic updates
	Matrix3& operator+=(const Matrix3& Matrix);
	Matrix3& operator-=(const Matrix3& Matrix);
	Matrix3& operator*=(const Matrix3& Matrix);
	Matrix3& operator*=(float fScalar);
	Matrix3& operator/=(float fScalar);

	// matrix - vector operations
	Vector3 operator*(const Vector3& rkV) const;  // M * v

private:
	float m_afEntry[3 * 3];
	static int I(int iRow, int iCol); // iRow*N + iCol
};

inline Matrix3::Matrix3(bool bZero)
{
	if (bZero)
		memset(m_afEntry, 0, 3 * 3 * sizeof(float));
}

inline Matrix3::Matrix3(const Matrix3& Matrix)
{
	memcpy(m_afEntry, (void*)&Matrix, 9 * sizeof(float));
}

inline Matrix3::Matrix3(float fM11, float fM12, float fM13, float fM21, float fM22, float fM23, float fM31, float fM32, float fM33)
{
	m_afEntry[0] = fM11;
	m_afEntry[1] = fM12;
	m_afEntry[2] = fM13;

	m_afEntry[3] = fM21;
	m_afEntry[4] = fM22;
	m_afEntry[5] = fM23;

	m_afEntry[6] = fM31;
	m_afEntry[7] = fM32;
	m_afEntry[8] = fM33;
}

inline void Matrix3::RotationX(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = 1.0f;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = 0.0f;

	m_afEntry[3] = 0.0f;
	m_afEntry[4] = fCos;
	m_afEntry[5] = fSin;

	m_afEntry[6] = 0.0f;
	m_afEntry[7] = -fSin;
	m_afEntry[8] = fCos;
}

inline void Matrix3::RotationY(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = fCos;
	m_afEntry[1] = 0.0f;
	m_afEntry[2] = -fSin;

	m_afEntry[3] = 0.0f;
	m_afEntry[4] = 1.0f;
	m_afEntry[5] = 0.0f;

	m_afEntry[6] = fSin;
	m_afEntry[7] = 0.0f;
	m_afEntry[8] = fCos;
}

inline void Matrix3::RotationZ(float fRadians)
{
	float fSin = sinf(fRadians);
	float fCos = cosf(fRadians);

	m_afEntry[0] = fCos;
	m_afEntry[1] = fSin;
	m_afEntry[2] = 0.0f;

	m_afEntry[3] = -fSin;
	m_afEntry[4] = fCos;
	m_afEntry[5] = 0.0f;

	m_afEntry[6] = 0.0f;
	m_afEntry[7] = 0.0f;
	m_afEntry[8] = 1.0f;
}

inline void Matrix3::Rotation(Vector3& Rot)
{
	Matrix3 mRot1;
	Matrix3 mRot2;

	mRot1.RotationZ(Rot.z);
	mRot2.RotationX(Rot.x);
	mRot1 *= mRot2;
	mRot2.RotationY(Rot.y);
	mRot1 *= mRot2;
	*this = mRot1;
}

inline void Matrix3::RotationZYX(Vector3& Rot)
{
	Matrix3 mRot1;
	Matrix3 mRot2;

	mRot1.RotationZ(Rot.z);
	mRot2.RotationY(Rot.y);
	mRot1 *= mRot2;
	mRot2.RotationX(Rot.x);
	mRot1 *= mRot2;
	*this = mRot1;
}

inline void Matrix3::RotationEuler(Vector3& Axis, float Angle)
{
	float s = sinf(Angle);
	float c = cosf(Angle);
	float t = 1 - c;

	m_afEntry[0] = t * Axis.x * Axis.x + c;
	m_afEntry[1] = t * Axis.x * Axis.y + s * Axis.z;
	m_afEntry[2] = t * Axis.x * Axis.z - s * Axis.y;

	m_afEntry[3] = t * Axis.x * Axis.y - s * Axis.z;
	m_afEntry[4] = t * Axis.y * Axis.y + c;
	m_afEntry[5] = t * Axis.y * Axis.z + s * Axis.x;

	m_afEntry[6] = t * Axis.x * Axis.y + s * Axis.y;
	m_afEntry[7] = t * Axis.y * Axis.z - s * Axis.x;
	m_afEntry[8] = t * Axis.z * Axis.z + c;
}

inline void Matrix3::Orthonormalize()
{
	// This method is taken from the Wild Magic library v3.11, available at
	// http://www.geometrictools.com.

	// Algorithm uses Gram-Schmidt orthogonalization.  If 'this' matrix is
	// M = [m0|m1|m2], then orthonormal output matrix is Q = [q0|q1|q2],
	//
	//   q0 = m0/|m0|
	//   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
	//   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
	//
	// where |V| indicates length of vector V and A*B indicates dot
	// product of vectors A and B.

	//(1.0/sqrt((double)fValue))

	// compute q0
	float fInvLength = static_cast<float>(1.0 / sqrt((double)(m_afEntry[0] * m_afEntry[0] +
		m_afEntry[3] * m_afEntry[3] + m_afEntry[6] * m_afEntry[6])));

	m_afEntry[0] *= fInvLength;
	m_afEntry[3] *= fInvLength;
	m_afEntry[6] *= fInvLength;

	// compute q1
	float fDot0 = m_afEntry[0] * m_afEntry[1] + m_afEntry[3] * m_afEntry[4] +
		m_afEntry[6] * m_afEntry[7];

	m_afEntry[1] -= fDot0 * m_afEntry[0];
	m_afEntry[4] -= fDot0 * m_afEntry[3];
	m_afEntry[7] -= fDot0 * m_afEntry[6];

	fInvLength = static_cast<float>(1.0 / sqrt((double)(m_afEntry[1] * m_afEntry[1] +
		m_afEntry[4] * m_afEntry[4] + m_afEntry[7] * m_afEntry[7])));

	m_afEntry[1] *= fInvLength;
	m_afEntry[4] *= fInvLength;
	m_afEntry[7] *= fInvLength;

	// compute q2
	float fDot1 = m_afEntry[1] * m_afEntry[2] + m_afEntry[4] * m_afEntry[5] +
		m_afEntry[7] * m_afEntry[8];

	fDot0 = m_afEntry[0] * m_afEntry[2] + m_afEntry[3] * m_afEntry[5] +
		m_afEntry[6] * m_afEntry[8];

	m_afEntry[2] -= fDot0 * m_afEntry[0] + fDot1 * m_afEntry[1];
	m_afEntry[5] -= fDot0 * m_afEntry[3] + fDot1 * m_afEntry[4];
	m_afEntry[8] -= fDot0 * m_afEntry[6] + fDot1 * m_afEntry[7];

	fInvLength = static_cast<float>(1.0 / sqrt((double)(m_afEntry[2] * m_afEntry[2] +
		m_afEntry[5] * m_afEntry[5] + m_afEntry[8] * m_afEntry[8])));

	m_afEntry[2] *= fInvLength;
	m_afEntry[5] *= fInvLength;
	m_afEntry[8] *= fInvLength;
}

inline void Matrix3::MakeZero()
{
	memset(m_afEntry, 0, 3 * 3 * sizeof(float));
}

inline void Matrix3::MakeIdentity()
{
	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
		{
			if (iRow == iCol)
				m_afEntry[I(iRow, iCol)] = 1.0f;
			else
				m_afEntry[I(iRow, iCol)] = 0.0f;
		}
	}
}

inline void Matrix3::MakeTranspose()
{
	Matrix3 mTranspose;

	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
			mTranspose.m_afEntry[I(iRow, iCol)] = m_afEntry[I(iCol, iRow)];
	}

	memcpy(m_afEntry, mTranspose.m_afEntry, 3 * 3 * sizeof(float));
}

inline Matrix3 Matrix3::Zero()
{
	Matrix3 mReturn = Matrix3(true);
	return(mReturn);
}

inline Matrix3 Matrix3::Identity()
{
	Matrix3 mIdent;

	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
		{
			if (iRow == iCol)
				mIdent.m_afEntry[I(iRow, iCol)] = 1.0f;
			else
				mIdent.m_afEntry[I(iRow, iCol)] = 0.0f;
		}
	}
	return(mIdent);
}

inline Matrix3 Matrix3::Transpose()
{
	Matrix3 mTranspose;

	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
			mTranspose.m_afEntry[I(iRow, iCol)] = m_afEntry[I(iCol, iRow)];
	}
	return(mTranspose);
}

inline Matrix3& Matrix3::operator=(const Matrix3& Matrix)
{
	memcpy(m_afEntry, Matrix.m_afEntry, 9 * sizeof(float));
	return *this;
}

inline float Matrix3::operator()(int iRow, int iCol) const
{
	return(m_afEntry[I(iRow, iCol)]);
}

inline float& Matrix3::operator()(int iRow, int iCol)
{
	return(m_afEntry[I(iRow, iCol)]);
}

inline float Matrix3::operator[](int iPos) const
{
	return(m_afEntry[iPos]);
}

inline float& Matrix3::operator[](int iPos)
{
	return(m_afEntry[iPos]);
}

inline void Matrix3::SetRow(int iRow, const Vector3& Vector)
{
	for (int iCol = 0; iCol < 3; iCol++)
		m_afEntry[I(iRow, iCol)] = Vector[iCol];
}

inline Vector3 Matrix3::GetRow(int iRow) const
{
	Vector3 vRow;
	for (int iCol = 0; iCol < 3; iCol++)
		vRow[iCol] = m_afEntry[I(iRow, iCol)];

	return(vRow);
}

inline void Matrix3::SetColumn(int iCol, const Vector3& Vector)
{
	for (int iRow = 0; iRow < 3; iRow++)
		m_afEntry[I(iRow, iCol)] = Vector[iRow];
}

inline Vector3 Matrix3::GetColumn(int iCol) const
{
	Vector3 vCol;
	for (int iRow = 0; iRow < 3; iRow++)
		vCol[iRow] = m_afEntry[I(iRow, iCol)];

	return(vCol);
}

inline bool Matrix3::operator==(const Matrix3& Matrix) const
{
	return(memcmp(m_afEntry, Matrix.m_afEntry, 3 * 3 * sizeof(float)) == 0);
}

inline bool Matrix3::operator!=(const Matrix3& Matrix) const
{
	return(memcmp(m_afEntry, Matrix.m_afEntry, 3 * 3 * sizeof(float)) != 0);
}

inline Matrix3 Matrix3::operator+(const Matrix3& Matrix) const
{
	Matrix3 mSum;

	for (int i = 0; i < 3 * 3; i++)
		mSum.m_afEntry[i] = m_afEntry[i] + Matrix.m_afEntry[i];

	return(mSum);
}

inline Matrix3 Matrix3::operator-(const Matrix3& Matrix) const
{
	Matrix3 mDiff;

	for (int i = 0; i < 3 * 3; i++)
		mDiff.m_afEntry[i] = m_afEntry[i] - Matrix.m_afEntry[i];

	return(mDiff);
}

inline Matrix3 Matrix3::operator*(const Matrix3& Matrix) const
{
	Matrix3 mProd;

	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
		{
			int i = I(iRow, iCol);
			mProd.m_afEntry[i] = 0.0f;
			for (int iMid = 0; iMid < 3; iMid++)
			{
				mProd.m_afEntry[i] +=
					m_afEntry[I(iRow, iMid)] * Matrix.m_afEntry[I(iMid, iCol)];
			}
		}
	}
	return(mProd);
}

inline Matrix3 Matrix3::operator*(float fScalar) const
{
	Matrix3 mProd;

	for (int i = 0; i < 3 * 3; i++)
		mProd.m_afEntry[i] = m_afEntry[i] * fScalar;

	return(mProd);
}

inline Matrix3 Matrix3::operator/(float fScalar) const
{
	Matrix3 mQuot;
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		for (int i = 0; i < 3 * 3; i++)
			mQuot.m_afEntry[i] = m_afEntry[i] * fInvScalar;
	}
	else
	{
		for (int i = 0; i < 3 * 3; i++)
			mQuot.m_afEntry[i] = 0;
	}

	return(mQuot);
}

inline Matrix3 Matrix3::operator-() const
{
	Matrix3 mNeg;

	for (int i = 0; i < 3 * 3; i++)
		mNeg.m_afEntry[i] = -m_afEntry[i];

	return(mNeg);
}

inline Matrix3& Matrix3::operator+=(const Matrix3& Matrix)
{
	for (int i = 0; i < 3 * 3; i++)
		m_afEntry[i] += Matrix.m_afEntry[i];

	return(*this);
}

inline Matrix3& Matrix3::operator-=(const Matrix3& Matrix)
{
	for (int i = 0; i < 3 * 3; i++)
		m_afEntry[i] -= Matrix.m_afEntry[i];

	return(*this);
}

inline Matrix3& Matrix3::operator*=(const Matrix3& Matrix)
{
	Matrix3 mProd = *this;

	for (int iRow = 0; iRow < 3; iRow++)
	{
		for (int iCol = 0; iCol < 3; iCol++)
		{
			int i = I(iRow, iCol);
			m_afEntry[i] = 0.0f;
			for (int iMid = 0; iMid < 3; iMid++)
			{
				m_afEntry[i] +=
					mProd.m_afEntry[I(iRow, iMid)] * Matrix.m_afEntry[I(iMid, iCol)];
			}
		}
	}
	return(*this);
}

inline Matrix3& Matrix3::operator*=(float fScalar)
{
	for (int i = 0; i < 3 * 3; i++)
		m_afEntry[i] *= fScalar;

	return(*this);
}

inline Matrix3& Matrix3::operator/=(float fScalar)
{
	if (fScalar != 0.0f)
	{
		float fInvScalar = 1.0f / fScalar;
		for (int i = 0; i < 3 * 3; i++)
			m_afEntry[i] *= fInvScalar;
	}
	else
	{
		for (int i = 0; i < 3 * 3; i++)
			m_afEntry[i] = 0;
	}

	return(*this);
}

inline Vector3 Matrix3::operator*(const Vector3& Vector) const
{
	Vector3 vProd;
	for (int iCol = 0; iCol < 3; iCol++)
	{
		vProd[iCol] = 0.0f;
		for (int iRow = 0; iRow < 3; iRow++)
			vProd[iCol] += m_afEntry[I(iRow, iCol)] * Vector[iRow];
	}
	return(vProd);
}

inline int Matrix3::I(int iRow, int iCol)
{
	return(3 * iRow + iCol);
}