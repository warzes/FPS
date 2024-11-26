#define ENABLE_SPIRV 1
#if defined(_WIN32)
#	define WIN32 1
#	define GLSLANG_OSINCLUDE_WIN32 1
#endif

// glslang lib

// OSDependent
#if defined(_WIN32)
#	include "glslang/glslang/OSDependent/Windows/ossource.cpp"
#endif