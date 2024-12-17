#include "stdafx.h"
#include "Transform3.h"
//=============================================================================
Transform3::Transform3()
{
	m_vTranslation = { 0.0f, 0.0f, 0.0f };
	m_mRotation.MakeIdentity();
	m_vScale = Vector3(1.0f, 1.0f, 1.0f);

	m_mWorld.MakeIdentity();
	m_mLocal.MakeIdentity();
}
//=============================================================================
Vector3& Transform3::Position()
{
	return(m_vTranslation);
}
//=============================================================================
Matrix3& Transform3::Rotation()
{
	return(m_mRotation);
}
//=============================================================================
Vector3& Transform3::Scale()
{
	return(m_vScale);
}
//=============================================================================
void Transform3::UpdateLocal()
{
	// Load the local space matrix with the rotation and translation components.

	m_mLocal.MakeIdentity();
	m_mLocal.SetRotation(m_mRotation);
	m_mLocal.SetTranslation(m_vTranslation);
	m_mLocal = Matrix4::ScaleMatrix(m_vScale) * m_mLocal;
}
//=============================================================================
void Transform3::UpdateWorld(const Matrix4& parent)
{
	m_mWorld = m_mLocal * parent;
}
//=============================================================================
void Transform3::UpdateWorld()
{
	// If no parent matrix is available, then simply make the world matrix the
	// local matrix.
	m_mWorld = m_mLocal;
}
//=============================================================================
const Matrix4& Transform3::WorldMatrix() const
{
	return(m_mWorld);
}
//=============================================================================
const Matrix4& Transform3::LocalMatrix() const
{
	return(m_mLocal);
}
//=============================================================================
Matrix4& Transform3::WorldMatrix()
{
	return(m_mWorld);
}
//=============================================================================
Matrix4& Transform3::LocalMatrix()
{
	return(m_mLocal);
}
//=============================================================================
Matrix4 Transform3::GetView() const
{
	Vector3 Eye;
	Vector3 At;
	Vector3 Up;

	Eye = m_mWorld.GetTranslation();
	At = m_mWorld.GetTranslation() + m_mWorld.GetBasisZ();
	Up = m_mWorld.GetBasisY();

	return(Matrix4::LookAtLHMatrix(Eye, At, Up));
}
//=============================================================================
Vector4 Transform3::LocalToWorldSpace(const Vector4& input)
{
	Vector4 result = WorldMatrix() * input;

	return(result);
}
//=============================================================================
Vector4 Transform3::WorldToLocalSpace(const Vector4& input)
{
	Vector4 result = WorldMatrix().Inverse() * input;

	return(result);
}
//=============================================================================
Vector3 Transform3::LocalVectorToWorldSpace(const Vector3& input)
{
	Vector4 result = LocalToWorldSpace(Vector4(input, 0.0f));

	return{ result.x, result.y, result.z };
}
//=============================================================================
Vector3 Transform3::LocalPointToWorldSpace(const Vector3& input)
{
	Vector4 result = LocalToWorldSpace(Vector4(input, 1.0f));

	return{ result.x, result.y, result.z };
}
//=============================================================================
Vector3 Transform3::WorldVectorToLocalSpace(const Vector3& input)
{
	Vector4 result = WorldToLocalSpace(Vector4(input, 0.0f));

	return{ result.x, result.y, result.z };
}
//=============================================================================
Vector3 Transform3::WorldPointToLocalSpace(const Vector3& input)
{
	Vector4 result = WorldToLocalSpace(Vector4(input, 1.0f));
	return{ result.x, result.y, result.z };
}
//--------------------------------------------------------------------------------