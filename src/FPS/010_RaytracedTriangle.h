#pragma once

void E010()
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

layout(binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 1, rgba8) uniform image2D image;

layout(location = 0) rayPayloadEXT vec3 hitValue;

void main()
{
	vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy);

	vec3 origin = vec3(uv.x, 1.0f - uv.y, -1.0f);
	vec3 direction = vec3(0.0f, 0.0f, 1.0f);

	uint rayFlags = gl_RayFlagsNoneEXT;
	uint cullMask = 0xFF;
	uint sbtRecordOffset = 0;
	uint sbtRecordStride = 0;
	uint missIndex = 0;
	float tmin = 0.0f;
	float tmax = 10.0f;
	const int payloadLocation = 0;

    hitValue = vec3(0.0);

	traceRayEXT(topLevelAS, rayFlags, cullMask, sbtRecordOffset, sbtRecordStride, missIndex,
		origin, tmin, direction, tmax, payloadLocation);

	imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 0.0));
})";

		const std::string miss_shader_code = R"(
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
hitAttributeEXT vec3 attribs;

void main()
{
	vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
	hitValue = barycentricCoords;
})";

		const std::vector<glm::vec3> vertices = {
			{ 0.25f, 0.25f, 0.0f },
			{ 0.75f, 0.25f, 0.0f },
			{ 0.50f, 0.75f, 0.0f },
		};

		const std::vector<uint32_t> indices = { 0, 1, 2 };

		auto shader = RaytracingShader(raygen_shader_code, { miss_shader_code }, closesthit_shader_code);
		auto blas = BLAS(vertices, 0, indices, 0, glm::mat4(1.0f));
		auto tlas = TLAS({ blas });

		auto& rhi = engine.GetRenderSystem();

		while (!engine.IsShouldClose())
		{
			engine.BeginFrame();

			auto target = rhi.AcquireTransientRenderTarget();

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