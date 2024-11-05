#include "stdafx.h"

#if defined(_MSC_VER)
#	pragma comment( lib, "Engine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif

bool IsRequestExit = false;
void RequestExit()
{
	IsRequestExit = true;
}

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	LogSystem log;
	log.Create({});

	WindowSystem window;
	window.Create({});

	while (!IsRequestExit)
	{
		window.PollEvent();
	}

	window.Destroy();
	log.Destroy();
}