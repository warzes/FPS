#define ENABLE_SPIRV 1
#if defined(_WIN32)
#	define WIN32 1
#	define GLSLANG_OSINCLUDE_WIN32 1
#endif

// glslang lib

// MachineIndependent
#include "glslang/glslang/MachineIndependent/Scan.cpp"
#include "glslang/glslang/MachineIndependent/ShaderLang.cpp"

// SPIRV
#include "glslang/SPIRV/CInterface/spirv_c_interface.cpp"
#include "glslang/SPIRV/disassemble.cpp"
#include "glslang/SPIRV/doc.cpp"
#include "glslang/SPIRV/GlslangToSpv.cpp"
#include "glslang/SPIRV/InReadableOrder.cpp"
#include "glslang/SPIRV/Logger.cpp"
#include "glslang/SPIRV/SpvBuilder.cpp"
#include "glslang/SPIRV/SpvPostProcess.cpp"
#include "glslang/SPIRV/SpvTools.cpp"
