#pragma once

#pragma warning(push, 3)
#pragma warning(disable : 4061)
#pragma warning(disable : 4365)
#pragma warning(disable : 4820)
#pragma warning(disable : 5039)

#define _USE_MATH_DEFINES

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#include <Windows.h>
//#include <ShellScalingApi.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>
#if defined(_DEBUG)
#	include <dxgidebug.h>
#endif
#include <d3dcompiler.h>

#undef near
#undef far

#include <stb/stb_image.h>

#include <cassert>
#include <algorithm>
#include <string>
#include <string_view>


#pragma warning(pop)