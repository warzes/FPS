#include "stdafx.h"
#if RENDER_OPENGL
#include "RenderSystem.h"
#include "ContextOpenGL.h"
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

	std::cout << "[opengl debug] name: " << source_str <<
		", type: " << type_str <<
		", id: " << id <<
		", severity: " << severity_str <<
		", message: " << message << std::endl;
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

		std::cout << "[opengl] error: " << name << " (" << error << ")" << std::endl;
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
	wglSwapIntervalEXT(value ? 1 : 0);
#endif

	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
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
#endif // RENDER_OPENGL