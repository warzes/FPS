﻿#pragma once

void E003()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		const std::string vertex_shader_code = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;
layout(location = TEXCOORD_LOCATION) in vec2 aTexCoord;

layout(location = 0) out struct { vec4 Color; vec2 TexCoord; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	Out.TexCoord = aTexCoord;
#ifdef FLIP_TEXCOORD_Y
	Out.TexCoord.y = 1.0 - Out.TexCoord.y;
#endif
	gl_Position = vec4(aPosition, 1.0);
})";

		const std::string fragment_shader_code = R"(
#version 450 core

layout(location = 0) out vec4 result;
layout(location = 0) in struct { vec4 Color; vec2 TexCoord; } In;

layout(binding = 0) uniform sampler2D sTexture;

void main()
{
	result = In.Color * texture(sTexture, In.TexCoord);
})";

		using Vertex = vertex::PositionColorTexture;

		const std::vector<Vertex> vertices = {
			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }, // bottom right
			{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }, // bottom left
			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } }, // top right
			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }, // top left
		};

		const std::vector<uint32_t> indices = { 0, 1, 2, 2, 1, 3 };
	

		auto shader = Shader(vertex_shader_code, fragment_shader_code, Vertex::Defines);

		auto [tex_width, tex_height, tex_memory] = LoadTexture("assets/bricks.jpg");

		auto texture = Texture(tex_width, tex_height, PixelFormat::RGBA8UNorm, tex_memory);

		auto& rhi = engine.GetRenderSystem();

		rhi.SetTopology(Topology::TriangleList);
		rhi.SetShader(shader);
		rhi.SetTexture(0, texture);
		rhi.SetVertexBuffer(vertices);
		rhi.SetIndexBuffer(indices);
		rhi.SetInputLayout(Vertex::Layout);

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();
			rhi.SetSize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());
			rhi.Clear();
			rhi.DrawIndexed(static_cast<uint32_t>(indices.size()));
			rhi.Present();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}