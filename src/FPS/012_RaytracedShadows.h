﻿#pragma once

void E012()
{
#if RENDER_VULKAN
	EngineApp engine;
	EngineAppCreateInfo eaci{};
	eaci.render.features = { RenderFeature::Raytracing };
	if (engine.Create(eaci))
	{
		const std::string raygen_shader_code = R"(
#version 460
#extension GL_EXT_ray_tracing : require

layout(binding = 2) uniform _settings
{
	mat4 viewInverse;
	mat4 projInverse;
} settings;

layout(binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 1, rgba8) uniform image2D image;

layout(location = 0) rayPayloadEXT vec3 hitValue;

void main()
{
	vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	vec2 uv = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
	vec2 d = uv * 2.0 - 1.0;

	vec4 origin = settings.viewInverse * vec4(0, 0, 0, 1);
	vec4 target = settings.projInverse * vec4(d.x, d.y, 1, 1);
	vec4 direction = settings.viewInverse * vec4(normalize(target.xyz), 0);

	uint rayFlags = gl_RayFlagsNoneEXT;
	uint cullMask = 0xFF;
	uint sbtRecordOffset = 0;
	uint sbtRecordStride = 0;
	uint missIndex = 0;
	float tmin = 0.001f;
	float tmax = 8192.0f;
	const int payloadLocation = 0;

    hitValue = vec3(0.0);

	traceRayEXT(topLevelAS, rayFlags, cullMask, sbtRecordOffset, sbtRecordStride, missIndex,
		origin.xyz, tmin, direction.xyz, tmax, payloadLocation);

	imageStore(image, ivec2(gl_LaunchSizeEXT.xy) - ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 0.0));
})";

		const std::string miss_shader_code = R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 1) rayPayloadInEXT bool isShadowed;

void main()
{
  isShadowed = false;
})";

		const std::string shadow_miss_shader_code = R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
	hitValue = vec3(0.0, 0.0, 0.0);
})";

		const std::string closesthit_shader_code = R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
layout(location = 1) rayPayloadEXT bool isShadowed;

hitAttributeEXT vec3 attribs;

layout(binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 3) uniform sampler2D tex;

layout(binding = 4) buffer Indices { uint i[]; } indices;
layout(binding = 5) buffer Vertices { vec4 v[]; } vertices;

layout(binding = 6) uniform _light
{
	vec3 direction;
} light;

struct Vertex
{
	vec3 pos;
	vec4 color;
	vec2 texcoord;
	vec3 normal;
};

uint vertexSize = 3; // number of vec4 values used to represent a vertex

Vertex unpackVertex(uint index)
{
	Vertex v;

	vec4 d0 = vertices.v[vertexSize * index + 0];
	vec4 d1 = vertices.v[vertexSize * index + 1];
	vec4 d2 = vertices.v[vertexSize * index + 2];

	v.pos = d0.xyz;
	v.color = vec4(d0.w, d1.xyz);
	v.texcoord = vec2(d1.w, d2.x);
	v.normal = vec3(d2.y, d2.z, d2.w);

	return v;
}

void main()
{
	uint index0 = indices.i[gl_PrimitiveID * 3 + 0];
	uint index1 = indices.i[gl_PrimitiveID * 3 + 1];
	uint index2 = indices.i[gl_PrimitiveID * 3 + 2];

	Vertex v0 = unpackVertex(index0);
	Vertex v1 = unpackVertex(index1);
	Vertex v2 = unpackVertex(index2);

	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	//vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);

    vec2 texcoord = v0.texcoord * barycentrics.x + v1.texcoord * barycentrics.y + v2.texcoord * barycentrics.z;
	hitValue = texture(tex, texcoord).xyz;

	float tmin = 0.001;
	float tmax = 1000.0f;
	vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	vec3 direction = normalize(light.direction);
	uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
	uint cullMask = 0xFF;
	uint sbtRecordOffset = 1;
	uint sbtRecordStride = 0;
	uint missIndex = 1;
	const int payloadLocation = 1;

	isShadowed = true;

	traceRayEXT(topLevelAS, rayFlags, cullMask, sbtRecordOffset, sbtRecordStride, missIndex,
		origin, tmin, direction, tmax, payloadLocation);

	if (isShadowed)
		hitValue *= 0.3f;
})";

		const auto WhiteColor = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		const std::vector<vertex::PositionColorTextureNormal> vertices = {
			/* front */
			/* 0  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 1  */ { {  1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 2  */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
			/* 3  */ { {  1.0f, -1.0f,  1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },

			/* top */
			/* 4  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 5  */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 6  */ { {  1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			/* 7  */ { {  1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },

			/* left */
			/* 8  */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 9  */ { { -1.0f,  1.0f,  1.0f }, WhiteColor, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 10 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
			/* 11 */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },

			/* back */
			/* 12 */ { { -1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 13 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 14 */ { {  1.0f,  1.0f, -1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			/* 15 */ { {  1.0f, -1.0f, -1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

			/* bottom */
			/* 16 */ { { -1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 17 */ { {  1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 18 */ { { -1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			/* 19 */ { {  1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

			/* right */
			/* 20 */ { { 1.0f, -1.0f, -1.0f }, WhiteColor, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 21 */ { { 1.0f, -1.0f,  1.0f }, WhiteColor, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 22 */ { { 1.0f,  1.0f, -1.0f }, WhiteColor, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			/* 23 */ { { 1.0f,  1.0f,  1.0f }, WhiteColor, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 1, 3, 2, // front
			4, 5, 6, 5, 7, 6, // top
			8, 9, 10, 9, 11, 10, // left
			12, 13, 14, 13, 15, 14, // back
			16, 17, 18, 17, 19, 18, // bottom
			20, 21, 22, 21, 23, 22, // right
		};

		struct alignas(16) Settings
		{
			glm::mat4 viewInverse;
			glm::mat4 projInverse;
		};

		struct alignas(16) Light
		{
			alignas(16) glm::vec3 direction;
		} light;

		auto shader = RaytracingShader(raygen_shader_code, { miss_shader_code, shadow_miss_shader_code }, closesthit_shader_code);

		auto [tex_width, tex_height, tex_memory] = LoadTextureFromSTBImage("assets/bricks.jpg");

		auto texture = Texture(tex_width, tex_height, PixelFormat::RGBA8UNorm, tex_memory, true);

		const auto yaw = 0.0f;
		const auto pitch = glm::radians(-25.0f);
		const auto position = glm::vec3{ -5.0f, 2.0f, 0.0f };

		light.direction = { -1.0f, 0.5f, 0.5f };

		auto& rhi = engine.GetRenderSystem();

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();

			static float time = 0.0;
			time += 0.001f;

			auto [view, projection] = utils::CalculatePerspectiveViewProjection(yaw, pitch, position, rhi.GetBackBufferWidth(), rhi.GetBackBufferHeight());

			auto model = glm::mat4(1.0f);
			model = glm::rotate(model, time, { 0.0f, 1.0f, 0.0f });
			auto blas = BLAS(vertices, 0, indices, 0, model);

			auto static_model = glm::mat4(1.0f);
			static_model = glm::scale(static_model, { 1.0f, 20.0f, 20.0f });
			static_model = glm::translate(static_model, { 5.0f, 0.0f, 0.0f });
			auto static_blas = BLAS(vertices, 0, indices, 0, static_model);

			auto tlas = TLAS({
				blas,
				static_blas,
				});

			rhi.SetUniformBuffer(2, Settings{
				.viewInverse = glm::inverse(view),
				.projInverse = glm::inverse(projection)
				});

			auto target = rhi.AcquireTransientRenderTarget();

			rhi.SetStorageBuffer(4, (void*)indices.data(), indices.size() * sizeof(uint32_t));
			rhi.SetStorageBuffer(5, (void*)vertices.data(), vertices.size() * sizeof(vertex::PositionColorTextureNormal));
			rhi.SetUniformBuffer(6, light);
			rhi.SetTexture(3, texture);
			rhi.SetShader(shader);
			rhi.SetRenderTarget(*target);
			rhi.SetAccelerationStructure(0, tlas);
			rhi.DispatchRays(target->GetWidth(), target->GetHeight(), 1);

			utils::passes::Blit(target, nullptr, {.clear = true});

			engine.EndFrame();
		}
	}
	engine.Destroy();
#endif // RENDER_VULKAN
}