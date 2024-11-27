#pragma once

#if defined(_ENGINE_PRIVATE_HEADER)

#if RENDER_D3D11

#	include <d3dcompiler.h>
#	include <d3d11.h>
#	include <dxgi1_6.h>

#	include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif // RENDER_D3D12

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

#if RENDER_OPENGL
#	if PLATFORM_WINDOWS
#		define GLEW_STATIC
#		include <GL/glew.h>
#		include <GL/wglew.h>
#	elif PLATFORM_EMSCRIPTEN
#		include <EGL/egl.h>
#		include <EGL/eglext.h>
#		include <EGL/eglplatform.h>
#		include <GLES3/gl3.h>
#	endif
#endif // RENDER_OPENGL

#if RENDER_VULKAN
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 0
#	include <vulkan/vulkan_raii.hpp>
#endif // RENDER_VULKAN


#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_hlsl.hpp>
#include <spirv_reflect.h>

#endif // _ENGINE_PRIVATE_HEADER