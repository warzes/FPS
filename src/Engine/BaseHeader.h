#pragma once

#if defined(_MSC_VER)
#	pragma warning(push, 3)
#	pragma warning(disable : 4820)
#	pragma warning(disable : 5039)
#endif

#define _USE_MATH_DEFINES

#include <cassert>
#include <cstdint>
#include <optional>
#include <variant>
#include <string>
#include <string_view>
#include <ranges>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <typeindex>

#if PLATFORM_WINDOWS

#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX

#	define NOBITMAP

#	define NOGDICAPMASKS
//#	define NOSYSMETRICS
#	define NOMENUS
#	define NOICONS
#	define NOSYSCOMMANDS
#	define NORASTEROPS
#	define OEMRESOURCE
#	define NOATOM
#	define NOCLIPBOARD
#	define NOCTLMGR
#	define NODRAWTEXT
#	define NOKERNEL
//#	define NONLS
#	define NOMEMMGR
#	define NOMETAFILE
#	define NOOPENFILE
#	define NOSCROLL
#	define NOSERVICE
#	define NOSOUND
#	define NOTEXTMETRIC
#	define NOWH
#	define NOCOMM
#	define NOKANJI
#	define NOHELP
#	define NOPROFILER
#	define NODEFERWINDOWPOS
#	define NOMCX
#	define NORPC
#	define NOPROXYSTUB
#	define NOIMAGE
#	define NOTAPE

#if !RENDER_OPENGL
#	define NOGDI
#endif

#	include <winsdkver.h>
//#	define _WIN32_WINNT 0x0601 // Windows 7
#	define _WIN32_WINNT 0x0A00 // Windows 10
#	include <sdkddkver.h>

#	include <Windows.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include <stb/stb_image.h>

#include "PrivateHeader.h"

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif