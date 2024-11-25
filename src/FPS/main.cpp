#include "stdafx.h"

#if defined(_MSC_VER)
#	pragma comment( lib, "Engine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	EngineApp engine;
	if (engine.Create({}))
	{
		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}