﻿#include "stdafx.h"
#if RENDER_OPENGL
#include "RenderSystem.h"
#include "ContextOpenGL.h"
#include "Log.h"
//=============================================================================
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
//=============================================================================
#if PLATFORM_WINDOWS
extern "C" {
	_declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	_declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif
//=============================================================================
#if defined(_DEBUG)
void GLAPIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	static const std::unordered_map<GLenum, std::string> SourceMap = {
		{ GL_DEBUG_SOURCE_API, "GL_DEBUG_SOURCE_API" },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "GL_DEBUG_SOURCE_WINDOW_SYSTEM" },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "GL_DEBUG_SOURCE_SHADER_COMPILER" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY, "GL_DEBUG_SOURCE_THIRD_PARTY" },
		{ GL_DEBUG_SOURCE_APPLICATION, "GL_DEBUG_SOURCE_APPLICATION" },
		{ GL_DEBUG_SOURCE_OTHER, "GL_DEBUG_SOURCE_OTHER" },
	};

	static const std::unordered_map<GLenum, std::string> TypeMap = {
		{ GL_DEBUG_TYPE_ERROR, "GL_DEBUG_TYPE_ERROR" },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_PORTABILITY, "GL_DEBUG_TYPE_PORTABILITY" },
		{ GL_DEBUG_TYPE_PERFORMANCE, "GL_DEBUG_TYPE_PERFORMANCE" },
		{ GL_DEBUG_TYPE_MARKER, "GL_DEBUG_TYPE_MARKER" },
		{ GL_DEBUG_TYPE_PUSH_GROUP, "GL_DEBUG_TYPE_PUSH_GROUP" },
		{ GL_DEBUG_TYPE_POP_GROUP, "GL_DEBUG_TYPE_POP_GROUP" },
		{ GL_DEBUG_TYPE_OTHER, "GL_DEBUG_TYPE_OTHER" },
	};

	static const std::unordered_map<GLenum, std::string> SeverityMap = {
		{ GL_DEBUG_SEVERITY_HIGH, "GL_DEBUG_SEVERITY_HIGH" },
		{ GL_DEBUG_SEVERITY_MEDIUM, "GL_DEBUG_SEVERITY_MEDIUM" },
		{ GL_DEBUG_SEVERITY_LOW, "GL_DEBUG_SEVERITY_LOW" },
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "GL_DEBUG_SEVERITY_NOTIFICATION" },
	};

	std::string source_str = "unknown";
	std::string type_str = "unknown";
	std::string severity_str = "unknown";

	if (SourceMap.contains(source))
		source_str = SourceMap.at(source);

	if (TypeMap.contains(type))
		type_str = TypeMap.at(type);

	if (SeverityMap.contains(severity))
		severity_str = SeverityMap.at(severity);

	std::string text = "[opengl debug] name: " + source_str + 
		", type: " + type_str +
		", id: " + std::to_string(id) +
		", severity: " + severity_str +
		", message: " + message;
	Error(text);
}
#endif // _DEBUG
//=============================================================================
void FlushErrors()
{
	static const std::unordered_map<GLenum, std::string> ErrorMap = {
		{ GL_INVALID_ENUM, "GL_INVALID_ENUM" }, // Set when an enumeration parameter is not legal.
		{ GL_INVALID_VALUE, "GL_INVALID_VALUE" }, // Set when a value parameter is not legal.
		{ GL_INVALID_OPERATION, "GL_INVALID_OPERATION" }, // Set when the state for a command is not legal for its given parameters.
#ifdef GL_STACK_OVERFLOW // emscripten
		{ GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW" }, // Set when a stack pushing operation causes a stack overflow.
#endif
#ifdef GL_STACK_UNDERFLOW // emscripten
		{ GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW" }, // Set when a stack popping operation occurs while the stack is at its lowest point.
#endif
		{ GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY" }, // Set when a memory allocation operation cannot allocate(enough) memory.
		{ GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION" }, // Set when reading or writing to a framebuffer that is not complete.
	};

	while (true)
	{
		auto error = glGetError();

		if (error == GL_NO_ERROR)
			break;

		auto name = ErrorMap.contains(error) ? ErrorMap.at(error) : "UNKNOWN";

		std::string message = "[opengl] error: " + name + " (" + std::to_string(error) + ")";
	}
}
//=============================================================================
#if PLATFORM_WINDOWS
HGLRC WglContext;
HDC gHDC;
#elif PLATFORM_EMSCRIPTEN
EGLDisplay gEglDisplay;
EGLSurface gEglSurface;
EGLContext gEglContext;
EGLConfig gEglConfig;
#endif
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RenderSystem::createAPI(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo)
{
#if PLATFORM_WINDOWS
	gHDC = GetDC(data.hwnd);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAlphaBits = 8;

	int nPixelFormat = ChoosePixelFormat(gHDC, &pfd);
	SetPixelFormat(gHDC, nPixelFormat, &pfd);

	WglContext = wglCreateContext(gHDC);
	wglMakeCurrent(gHDC, WglContext);

	glewInit();

	const int pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 1,
		0
	};

	int pixelFormatID;
	UINT numFormats;
	wglChoosePixelFormatARB(gHDC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	DescribePixelFormat(gHDC, pixelFormatID, sizeof(pfd), &pfd);
	SetPixelFormat(gHDC, pixelFormatID, &pfd);

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, /*WGL_CONTEXT_DEBUG_BIT_ARB*/0,
		0
	};

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(WglContext);
	WglContext = wglCreateContextAttribsARB(gHDC, 0, attribs);
	wglMakeCurrent(gHDC, WglContext);
#elif PLATFORM_EMSCRIPTEN
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};
	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	gEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(gEglDisplay, NULL, NULL);
	EGLint num_configs;
	eglChooseConfig(gEglDisplay, attribs, &gEglConfig, 1, &num_configs);
	gEglSurface = eglCreateWindowSurface(gEglDisplay, gEglConfig, (EGLNativeWindowType)window, NULL);
	gEglContext = eglCreateContext(gEglDisplay, gEglConfig, NULL, context_attribs);
	eglMakeCurrent(gEglDisplay, gEglSurface, gEglSurface, gEglContext);
#endif

#if defined(_DEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	glDebugMessageCallback(DebugMessageCallback, nullptr);
#endif

	GLint num_extensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

	for (GLint i = 0; i < num_extensions; i++)
	{
		auto extension = glGetStringi(GL_EXTENSIONS, i);
		//	std::cout << extension << std::endl;
	}

	gContext.width = data.width;
	gContext.height = data.height;

#if PLATFORM_WINDOWS
	wglSwapIntervalEXT(createInfo.vsync ? 1 : 0);
#endif

	if (!gContext.Create()) return false;

	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
	gContext.Destroy();
#if PLATFORM_WINDOWS
	wglDeleteContext(WglContext);
#endif
}
//=============================================================================
void RenderSystem::resize(uint32_t width, uint32_t height)
{
	gContext.width = width;
	gContext.height = height;

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::present()
{
	FlushErrors();
#if PLATFORM_WINDOWS
	SwapBuffers(gHDC);
#elif PLATFORM_EMSCRIPTEN
	eglSwapBuffers(gEglDisplay, gEglSurface);
#endif
	gContext.execute_after_present.flush();
}
//=============================================================================
//=============================================================================
TextureHandle* RenderSystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureGL(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RenderSystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureGL*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RenderSystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RenderSystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* RenderSystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureGL*)texture_handle;
	auto render_target = new RenderTargetGL(texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RenderSystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetGL*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* RenderSystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderGL(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RenderSystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderGL*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* RenderSystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferGL(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (VertexBufferGL*)handle;
		delete buffer;
		});
}
//=============================================================================
void RenderSystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* RenderSystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferGL(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (IndexBufferGL*)handle;

		if (gContext.index_buffer == buffer)
			gContext.index_buffer = nullptr;

		delete buffer;
		});
}
//=============================================================================
void RenderSystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* RenderSystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferGL(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (UniformBufferGL*)handle;
		delete buffer;
		});
}
//=============================================================================
void RenderSystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferGL*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RenderSystem::SetTopology(Topology topology)
{
	static const std::unordered_map<Topology, GLenum> TopologyMap = {
	{ Topology::PointList, GL_POINTS },
	{ Topology::LineList, GL_LINES },
	{ Topology::LineStrip, GL_LINE_STRIP },
	{ Topology::TriangleList, GL_TRIANGLES },
	{ Topology::TriangleStrip, GL_TRIANGLE_STRIP }
	};

	gContext.topology = TopologyMap.at(topology);
}
//=============================================================================
void RenderSystem::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::SetScissor(std::optional<Scissor> scissor)
{
	gContext.scissor = scissor;
	gContext.scissor_dirty = true;
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, TextureHandle* handle)
{
	gContext.textures[binding] = (TextureGL*)handle;
	gContext.dirty_textures.insert(binding);
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, const TextureHandle* handle)
{
	gContext.textures[binding] = (TextureGL*)handle;
	gContext.dirty_textures.insert(binding);
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	if (count == 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		gContext.render_targets.clear();

		if (!gContext.viewport.has_value())
			gContext.viewport_dirty = true;

		return;
	}
	std::vector<RenderTargetGL*> render_targets;

	for (size_t i = 0; i < count; i++)
	{
		auto target = (RenderTargetGL*)(RenderTargetHandle*)*(RenderTarget*)render_target[i];
		render_targets.push_back(target);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, render_targets.at(0)->GetGLFramebuffer());

	for (size_t i = 0; i < render_targets.size(); i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)i, GL_TEXTURE_2D,
			render_targets.at(i)->GetTexture()->GetGLTexture(), 0);
	}

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);

	glDrawBuffers((GLsizei)render_targets.size(), gContext.draw_buffers.data());

	gContext.render_targets = render_targets;

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(ShaderHandle* handle)
{
	gContext.shader = (ShaderGL*)handle;
	gContext.shader_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(const ShaderHandle* handle)
{
	gContext.shader = (ShaderGL*)handle;
	gContext.shader_dirty = true;
}
//=============================================================================
void RenderSystem::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.input_layouts = value;
	gContext.vertex_array_dirty = true;
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	gContext.vertex_buffers.clear();
	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferGL*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		gContext.vertex_buffers.push_back(buffer);
	}
	gContext.vertex_array_dirty = true;
}
//=============================================================================
void RenderSystem::SetIndexBuffer(IndexBufferHandle* handle)
{
	gContext.index_buffer = (IndexBufferGL*)handle;
	gContext.index_buffer_dirty = true;
}
//=============================================================================
void RenderSystem::SetIndexBuffer(const IndexBufferHandle* handle)
{
	gContext.index_buffer = (IndexBufferGL*)handle;
	gContext.index_buffer_dirty = true;
}
//=============================================================================
void RenderSystem::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferGL*)handle;
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer->GetGLBuffer());
}
//=============================================================================
void RenderSystem::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	if (!blend_mode.has_value())
	{
		glDisable(GL_BLEND);
		return;
	}

	const static std::unordered_map<Blend, GLenum> BlendMap = {
		{ Blend::One, GL_ONE },
		{ Blend::Zero, GL_ZERO },
		{ Blend::SrcColor, GL_SRC_COLOR },
		{ Blend::InvSrcColor, GL_ONE_MINUS_SRC_COLOR },
		{ Blend::SrcAlpha, GL_SRC_ALPHA },
		{ Blend::InvSrcAlpha, GL_ONE_MINUS_SRC_ALPHA },
		{ Blend::DstColor, GL_DST_COLOR },
		{ Blend::InvDstColor, GL_ONE_MINUS_DST_COLOR },
		{ Blend::DstAlpha, GL_DST_ALPHA },
		{ Blend::InvDstAlpha, GL_ONE_MINUS_DST_ALPHA }
	};

	const static std::unordered_map<BlendFunction, GLenum> BlendOpMap = {
		{ BlendFunction::Add, GL_FUNC_ADD },
		{ BlendFunction::Subtract, GL_FUNC_SUBTRACT },
		{ BlendFunction::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT },
		{ BlendFunction::Min, GL_MIN },
		{ BlendFunction::Max, GL_MAX },
	};

	const auto& blend = blend_mode.value();

	glEnable(GL_BLEND);
	glBlendEquationSeparate(BlendOpMap.at(blend.colorFunc), BlendOpMap.at(blend.alphaFunc));
	glBlendFuncSeparate(BlendMap.at(blend.colorSrc), BlendMap.at(blend.colorDst),
		BlendMap.at(blend.alphaSrc), BlendMap.at(blend.alphaDst));
	glColorMask(blend.colorMask.red, blend.colorMask.green, blend.colorMask.blue, blend.colorMask.alpha);
}
//=============================================================================
void RenderSystem::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	if (!depth_mode.has_value())
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(true);
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(ComparisonFuncMap.at(depth_mode.value().func));
	glDepthMask(depth_mode.value().writeMask);
}
//=============================================================================
void RenderSystem::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	if (!stencil_mode.has_value())
	{
		glDisable(GL_STENCIL_TEST);
		return;
	}

	static const std::unordered_map<StencilOp, GLenum> StencilOpMap = {
		{ StencilOp::Keep, GL_KEEP },
		{ StencilOp::Zero, GL_ZERO },
		{ StencilOp::Replace, GL_REPLACE },
		{ StencilOp::IncrementSaturation, GL_INCR },
		{ StencilOp::DecrementSaturation, GL_DECR },
		{ StencilOp::Invert, GL_INVERT },
		{ StencilOp::Increment, GL_INCR_WRAP },
		{ StencilOp::Decrement, GL_DECR_WRAP },
	};

	auto stencil_mode_nn = stencil_mode.value();

	glEnable(GL_STENCIL_TEST);
	glStencilMask(stencil_mode_nn.writeMask);
	glStencilOp(StencilOpMap.at(stencil_mode_nn.failOp), StencilOpMap.at(stencil_mode_nn.depthFailOp), StencilOpMap.at(stencil_mode_nn.passOp));
	glStencilFunc(ComparisonFuncMap.at(stencil_mode_nn.func), stencil_mode_nn.reference, stencil_mode_nn.readMask);
}
//=============================================================================
void RenderSystem::SetCullMode(CullMode cull_mode)
{
	if (cull_mode == CullMode::None)
	{
		glDisable(GL_CULL_FACE);
		return;
	}

	static const std::unordered_map<CullMode, GLenum> CullMap = {
		{ CullMode::None, GL_NONE },
		{ CullMode::Front, GL_FRONT },
		{ CullMode::Back, GL_BACK }
	};

	glEnable(GL_CULL_FACE);
	glCullFace(CullMap.at(cull_mode));
}
//=============================================================================
void RenderSystem::SetSampler(Sampler value)
{
	gContext.sampler_state.sampler = value;
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetTextureAddress(TextureAddress value)
{
	gContext.sampler_state.textureAddress = value;
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetFrontFace(FrontFace value)
{
	gContext.front_face = value;
	gContext.front_face_dirty = true;
}
//=============================================================================
void RenderSystem::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
	if (!depth_bias.has_value())
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		return;
	}

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(depth_bias->factor, depth_bias->units);
}
//=============================================================================
void RenderSystem::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	auto scissor_was_enabled = glIsEnabled(GL_SCISSOR_TEST);

	if (scissor_was_enabled)
	{
		glDisable(GL_SCISSOR_TEST);
	}

	GLbitfield flags = 0;

	if (color.has_value())
	{
		flags |= GL_COLOR_BUFFER_BIT;
		auto _color = color.value();
		glClearColor(_color.r, _color.g, _color.b, _color.a);
	}

	if (depth.has_value())
	{
		flags |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf(depth.value());
	}

	if (stencil.has_value())
	{
		flags |= GL_STENCIL_BUFFER_BIT;
		glClearStencil(stencil.value());
	}

	GLboolean depth_mask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);

	if (!depth_mask)
	{
		glDepthMask(true);
	}

	glClear(flags);

	if (!depth_mask)
	{
		glDepthMask(GL_FALSE);
	}

	if (scissor_was_enabled)
	{
		glEnable(GL_SCISSOR_TEST);
	}
}
//=============================================================================
void RenderSystem::Draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count)
{
	EnsureGraphicsState(false);
	auto mode = gContext.topology;
	auto first = (GLint)vertex_offset;
	auto count = (GLsizei)vertex_count;
	auto primcount = (GLsizei)instance_count;
	glDrawArraysInstanced(mode, first, count, primcount);
}
//=============================================================================
void RenderSystem::DrawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count)
{
	EnsureGraphicsState(true);
	auto mode = gContext.topology;
	auto count = (GLsizei)index_count;
	auto index_size = gContext.index_buffer->GetStride();
	auto type = index_size == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	auto indices = (void*)(size_t)(index_offset * index_size);
	auto primcount = (GLsizei)instance_count;
	glDrawElementsInstanced(mode, count, type, indices, primcount);
}
//=============================================================================
void RenderSystem::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dst_texture_handle)
{
	auto dst_texture = (TextureGL*)dst_texture_handle;
	auto format = gContext.GetBackbufferFormat();

	assert(dst_texture->GetWidth() == size.x);
	assert(dst_texture->GetHeight() == size.y);
	assert(dst_texture->GetFormat() == format);

	if (size.x <= 0 || size.y <= 0)
		return;

	auto backbuffer_height = gContext.GetBackbufferHeight();

	auto x = (GLint)pos.x;
	auto y = (GLint)(backbuffer_height - pos.y - size.y);
	auto width = (GLint)size.x;
	auto height = (GLint)size.y;

	auto channels_count = GetFormatChannelsCount(format);
	auto channel_size = GetFormatChannelSize(format);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, gContext.pixel_buffer);
	glBufferData(GL_PIXEL_PACK_BUFFER, width * height * channels_count * channel_size, NULL, GL_STATIC_READ);
	glReadPixels(x, y, width, height, TextureFormatMap.at(format), PixelFormatTypeMap.at(format), 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	auto binding = TextureGL::ScopedBind(dst_texture->GetGLTexture());
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, gContext.pixel_buffer);

	glTexImage2D(GL_TEXTURE_2D, 0, TextureInternalFormatMap.at(format), width, height, 0,
		TextureFormatMap.at(format), PixelFormatTypeMap.at(format), NULL);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
//=============================================================================
#endif // RENDER_OPENGL