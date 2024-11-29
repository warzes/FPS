#pragma once

void E002()
{
	EngineApp engine;
	if (engine.Create({}))
	{
		const std::string vertex_shader_code = R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = COLOR_LOCATION) in vec4 aColor;

layout(binding = 0) uniform _ubo
{
	mat4 projection;
	mat4 view;
	mat4 model;
} ubo;

layout(location = 0) out struct { vec4 Color; } Out;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	Out.Color = aColor;
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPosition, 1.0);
}
)";

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

		struct alignas(16) Matrices
		{
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
		} matrices;

		auto shader = Shader(vertex_shader_code, fragment_shader_code, Vertex::Defines);

		auto& rhi = engine.GetRenderSystem();

		rhi.SetTopology(Topology::TriangleList);
		rhi.SetShader(shader);
		rhi.SetVertexBuffer(vertices);
		rhi.SetIndexBuffer(indices);
		rhi.SetInputLayout(Vertex::Layout);

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();

			static float time = 0.0;
			time += 0.001f;
			matrices.model = glm::mat4(1.0f);
			matrices.model = glm::translate(matrices.model, { glm::sin(time * 2.0f) * 0.5f, 0.0f, 0.0f });
			rhi.SetUniformBuffer(0, matrices);


			rhi.Resize(engine.GetWindowSystem().GetWidth(), engine.GetWindowSystem().GetHeight());

			rhi.Clear();
			rhi.DrawIndexed(static_cast<uint32_t>(indices.size()));

			engine.EndFrame();
		}
	}
	engine.Destroy();
}