#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"
#include "Matrix4.h"

class Transform3 final
{
public:
	Transform3();

	Vector3& Position();
	Matrix3& Rotation();
	Vector3& Scale();

	void UpdateLocal();
	void UpdateWorld(const Matrix4& parent);
	void UpdateWorld();

	const Matrix4& LocalMatrix() const;
	const Matrix4& WorldMatrix() const;
	Matrix4& LocalMatrix();
	Matrix4& WorldMatrix();

	Matrix4 GetView() const;

	Vector4 LocalToWorldSpace(const Vector4& input);
	Vector4 WorldToLocalSpace(const Vector4& input);
	Vector3 LocalVectorToWorldSpace(const Vector3& input);
	Vector3 LocalPointToWorldSpace(const Vector3& input);
	Vector3 WorldVectorToLocalSpace(const Vector3& input);
	Vector3 WorldPointToLocalSpace(const Vector3& input);

private:
	Vector3 m_vTranslation;	// The translation and rotation varaibles are updated
	Matrix3 m_mRotation;	// during the update phase and used to generate the
	Vector3 m_vScale;		// local matrix.  Then, the world matrix is updated
	Matrix4 m_mWorld;		// with the new local matrix and the entity's parent
	Matrix4 m_mLocal;		// world matrix.

};