#pragma once

#include "ImguiHelper.h"

void E008()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		auto imgui = ImguiHelper();

		auto& rhi = engine.GetRenderSystem();

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();
			rhi.SetSize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());

			static float time = 0.0f;
			time += 0.01f;

			rhi.Clear();

			
			rhi.Present();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}