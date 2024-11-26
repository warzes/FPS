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
		const std::string vertex_shader_code = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;

layout(location = 0) out struct { vec4 Color; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	gl_Position = vec4(aPosition, 1.0);
})";

		const std::string fragment_shader_code = R"(
#version 450 core

layout(location = 0) out vec4 result;
layout(location = 0) in struct { vec4 Color; } In;

void main()
{
	result = In.Color;
})";

		using Vertex = vertex::PositionColor;

		const std::vector<Vertex> vertices = {
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		};

		const std::vector<uint32_t> indices = { 0, 1, 2 };

		auto shader = Shader(vertex_shader_code, fragment_shader_code, Vertex::Defines);

		engine.GetRenderSystem().SetTopology(Topology::TriangleList);
		engine.GetRenderSystem().SetShader(shader);
		engine.GetRenderSystem().SetIndexBuffer(indices);
		engine.GetRenderSystem().SetVertexBuffer(vertices);
		engine.GetRenderSystem().SetInputLayout(Vertex::Layout);


		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();

			engine.GetRenderSystem().Clear();
			engine.GetRenderSystem().DrawIndexed(static_cast<uint32_t>(indices.size()));
			engine.GetRenderSystem().Present();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}