﻿#pragma once

void E007()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		const std::string vertex_shader_code = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;
layout(location = 2) in mat4 aModel;

layout(binding = 0) uniform _ubo
{
	mat4 projection;
	mat4 view;
} ubo;

layout(location = 0) out struct { vec4 Color; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	gl_Position = ubo.projection * ubo.view * aModel * vec4(aPosition, 1.0);
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

		const glm::vec4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
		const glm::vec4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
		const glm::vec4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		const glm::vec4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
		const glm::vec4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

		const std::vector<Vertex> vertices = {
			/* front */
			/* 0  */ { { -1.0f,  1.0f,  1.0f }, Red },
			/* 1  */ { {  1.0f,  1.0f,  1.0f }, Red },
			/* 2  */ { { -1.0f, -1.0f,  1.0f }, Red },
			/* 3  */ { {  1.0f, -1.0f,  1.0f }, Red },

			/* top */
			/* 4  */ { { -1.0f,  1.0f,  1.0f }, Green },
			/* 5  */ { { -1.0f,  1.0f, -1.0f }, Green },
			/* 6  */ { {  1.0f,  1.0f,  1.0f }, Green },
			/* 7  */ { {  1.0f,  1.0f, -1.0f }, Green },

			/* left */
			/* 8  */ { { -1.0f,  1.0f, -1.0f }, Blue },
			/* 9  */ { { -1.0f,  1.0f,  1.0f }, Blue },
			/* 10 */ { { -1.0f, -1.0f, -1.0f }, Blue },
			/* 11 */ { { -1.0f, -1.0f,  1.0f }, Blue },

			/* back */
			/* 12 */ { { -1.0f,  1.0f, -1.0f }, Yellow },
			/* 13 */ { { -1.0f, -1.0f, -1.0f }, Yellow },
			/* 14 */ { {  1.0f,  1.0f, -1.0f }, Yellow },
			/* 15 */ { {  1.0f, -1.0f, -1.0f }, Yellow },

			/* bottom */
			/* 16 */ { { -1.0f, -1.0f,  1.0f }, Cyan },
			/* 17 */ { {  1.0f, -1.0f,  1.0f }, Cyan },
			/* 18 */ { { -1.0f, -1.0f, -1.0f }, Cyan },
			/* 19 */ { {  1.0f, -1.0f, -1.0f }, Cyan },

			/* right */
			/* 20 */ { { 1.0f, -1.0f, -1.0f }, Magenta },
			/* 21 */ { { 1.0f, -1.0f,  1.0f }, Magenta },
			/* 22 */ { { 1.0f,  1.0f, -1.0f }, Magenta },
			/* 23 */ { { 1.0f,  1.0f,  1.0f }, Magenta },
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 1, 3, 2, // front
			4, 5, 6, 5, 7, 6, // top
			8, 9, 10, 9, 11, 10, // left
			12, 13, 14, 13, 15, 14, // back
			16, 17, 18, 17, 19, 18, // bottom
			20, 21, 22, 21, 23, 22, // right
		};

		struct alignas(16) Matrices
		{
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
		} matrices;

		auto shader = Shader(vertex_shader_code, fragment_shader_code, Vertex::Defines);

		constexpr int Width = 10;
		constexpr int Height = 10;

		std::vector<glm::mat4> model_matrices;
		model_matrices.resize(Width* Height);

		auto vertices_buffer = VertexBuffer(vertices);
		auto model_matrices_buffer = VertexBuffer(model_matrices);

		auto model_matrices_layout = InputLayout(InputLayout::Rate::Instance, {
			{ 2, { VertexFormat::Float4, 0 } },
			{ 3, { VertexFormat::Float4, 16 } },
			{ 4, { VertexFormat::Float4, 32 } },
			{ 5, { VertexFormat::Float4, 48 } }
			});

		auto yaw = 0.0f;
		auto pitch = glm::radians(-25.0f);
		auto position = glm::vec3{ -30.0f, 17.0f, 0.0f };

		auto& rhi = engine.GetRenderSystem();

		rhi.SetTopology(Topology::TriangleList);
		rhi.SetShader(shader);
		rhi.SetIndexBuffer(indices);
		rhi.SetCullMode(CullMode::Back);
		rhi.SetDepthMode(ComparisonFunc::Less);
		rhi.SetVertexBuffer({ &vertices_buffer, &model_matrices_buffer });
		rhi.SetInputLayout({ Vertex::Layout, model_matrices_layout });
		
		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();
			rhi.SetSize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());

			static float time = 0.0f;
			time += 0.01f;

			std::tie(matrices.view, matrices.projection) = CalculatePerspectiveViewProjection(yaw, pitch, position, rhi.GetBackbufferWidth(), rhi.GetBackbufferHeight());

			rhi.SetUniformBuffer(0, matrices);


			rhi.Clear();

			for (int i = 0; i < Width; i++)
			{
				for (int j = 0; j < Height; j++)
				{
					constexpr float Scale = 3.0f;

					float y = ((float)j - ((float)(Height - 1) / 2.0f)) * Scale;
					float z = ((float)i - ((float)(Width - 1) / 2.0f)) * Scale;

					auto model = glm::mat4(1.0f);
					model = glm::translate(model, { 0.0f, y, z });
					model = glm::rotate(model, time, { 0.0f, 1.0f, 0.0f });

					model_matrices[(i * Width) + j] = model;
				}
			}

			model_matrices_buffer.Write(model_matrices);

			rhi.DrawIndexed((uint32_t)indices.size(), 0, (uint32_t)model_matrices.size());
			rhi.Present();

			engine.EndFrame();
		}
	}
	engine.Destroy();
}