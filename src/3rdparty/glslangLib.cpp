#define ENABLE_SPIRV 1
#if defined(_WIN32)
#	define WIN32 1
#	define GLSLANG_OSINCLUDE_WIN32 1
#endif

// glslang lib

// CInterface
#include "glslang/glslang/CInterface/glslang_c_interface.cpp"

// GenericCodeGen
#include "glslang/glslang/GenericCodeGen/CodeGen.cpp"
#include "glslang/glslang/GenericCodeGen/Link.cpp"

// MachineIndependent
#include "glslang/glslang/MachineIndependent/preprocessor/Pp.cpp"
#include "glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp"
#include "glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp"
#include "glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp"
#include "glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp"
#include "glslang/glslang/MachineIndependent/attribute.cpp"
#include "glslang/glslang/MachineIndependent/Constant.cpp"
#include "glslang/glslang/MachineIndependent/glslang_tab.cpp"
#include "glslang/glslang/MachineIndependent/InfoSink.cpp"
#include "glslang/glslang/MachineIndependent/Initialize.cpp"
#include "glslang/glslang/MachineIndependent/Intermediate.cpp"
#include "glslang/glslang/MachineIndependent/intermOut.cpp"
#include "glslang/glslang/MachineIndependent/IntermTraverse.cpp"
#include "glslang/glslang/MachineIndependent/iomapper.cpp"
#include "glslang/glslang/MachineIndependent/limits.cpp"
#include "glslang/glslang/MachineIndependent/linkValidate.cpp"
#include "glslang/glslang/MachineIndependent/parseConst.cpp"
#include "glslang/glslang/MachineIndependent/ParseContextBase.cpp"
#include "glslang/glslang/MachineIndependent/ParseHelper.cpp"
#include "glslang/glslang/MachineIndependent/PoolAlloc.cpp"
#include "glslang/glslang/MachineIndependent/propagateNoContraction.cpp"
#include "glslang/glslang/MachineIndependent/reflection.cpp"
#include "glslang/glslang/MachineIndependent/RemoveTree.cpp"
//#include "glslang/glslang/MachineIndependent/Scan.cpp"         // => glslangLib2.cpp
//#include "glslang/glslang/MachineIndependent/ShaderLang.cpp"   // => glslangLib2.cpp
#include "glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp"
#include "glslang/glslang/MachineIndependent/SymbolTable.cpp"
#include "glslang/glslang/MachineIndependent/Versions.cpp"


// glslang-default-resource-limits lib
#include "glslang/glslang/ResourceLimits/resource_limits_c.cpp"
#include "glslang/glslang/ResourceLimits/ResourceLimits.cpp"



