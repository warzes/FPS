﻿#pragma once

#if defined(_ENGINE_PRIVATE_HEADER)

#if RENDER_D3D12
#	include <d3dcompiler.h>
#	include <d3d12.h>
#	include <dxgi1_6.h>
#	include <dxgidebug.h>

#	include <d3dx12/d3dx12.h>
#	include <d3dx12/d3d12generatemips.h>
#	include <d3dx12/DirectXHelpers.h>
#	include <d3dx12/PlatformHelpers.h>

#	include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif // RENDER_D3D12

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_hlsl.hpp>
#include <spirv_reflect.h>

#endif // _ENGINE_PRIVATE_HEADER