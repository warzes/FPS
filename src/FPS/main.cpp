#include "stdafx.h"
//=============================================================================
#define EXAMPLE 1
#if EXAMPLE
#include "001_triangle.h"
#include "002_uniform.h"
#include "003_texture.h"
#include "004_texture_mipmap.h"
#include "005_LightCube.h"
#include "006_render_target.h"
#include "007_instancing.h"
#include "008_bloom.h"
#include "009_sponza.h"
#endif // EXAMPLE
//=============================================================================
#if defined(_MSC_VER)
#	pragma comment( lib, "Engine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif
//=============================================================================
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
#if EXAMPLE
	E001();
	//E002();
	//E003();
	//E004();
	//E005();
	//E006();
	//E007();
	//E008();
	//E009();

#else
	void GameStart();
	GameStart();
#endif // EXAMPLE
}
//=============================================================================