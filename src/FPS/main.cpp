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
#include "010_RaytracedTriangle.h"
#include "011_raytracedCube.h"
#include "012_RaytracedShadows.h"
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
https://www.3dgep.com/introduction-to-directx-11/
	Adria d2d12
		EveryRay - Rendering - Engine

#if EXAMPLE
	//E001();
	E002(); 
	//E003();
	//E004();
	//E005();
	//E006();
	//E007();
	//E008();
	//E009();
	//E010();
	//E011();
	//E012();
#else
	void GameStart();
	GameStart();
#endif // EXAMPLE
}
//=============================================================================