#pragma once

#pragma warning(push, 3)
#pragma warning(disable : 4061)
#pragma warning(disable : 4365)
#pragma warning(disable : 4820)
#pragma warning(disable : 5039)

#define _USE_MATH_DEFINES

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
//#include <ShellScalingApi.h>

#include <d3d11_4.h>
//#include <d3dcompiler.h>

#undef near
#undef far

#include <cassert>
#include <string>
#include <string_view>

#pragma warning(pop)