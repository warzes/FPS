#include "stdafx.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "Matrix3.h"
#include "Matrix4.h"

#include "AxisAlignedBox.h"
#include "Box3.h"
#include "Plane3.h"
#include "Ray3.h"
#include "Sphere.h"
#include "Frustum.h"

const Vector3 Vector3::Zero = { 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::One = { 1.0f, 1.0f, 1.0f };
const Vector3 Vector3::Forward = { 0.0f, 0.0f, -1.0f }; // TODO: это для правосторонней системы координат, нужно сделать чтобы настраивалось
const Vector3 Vector3::Backward = { 0.0f, 0.0f, 1.0f }; // TODO: это для правосторонней системы координат, нужно сделать чтобы настраивалось
const Vector3 Vector3::Up = { 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::Down = { 0.0f, -1.0f, 0.0f };
const Vector3 Vector3::Right = { 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::Left = { -1.0f, 0.0f, 0.0f };

const Matrix4 Matrix4::Identity = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f };

const Matrix4 Matrix4::Zero = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f };