#pragma once

#pragma warning(push, 3)
#pragma warning(disable : 4061)
#pragma warning(disable : 4365)
#pragma warning(disable : 4820)
#pragma warning(disable : 5039)

#define _USE_MATH_DEFINES

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
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
#include <wrl.h>
#include <wincodec.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#define _XM_NO_XMVECTOR_OVERLOADS_
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#undef near
#undef far

#include <stb/stb_image.h>

#include <cassert>
#include <algorithm>
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <mutex>

//#include <ppl.h>
//#include <ppltasks.h>

#pragma warning(pop)