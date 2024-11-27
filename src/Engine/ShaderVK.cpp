#include "stdafx.h"
#if RENDER_VULKAN
#include "ShaderVK.h"
#include "ContextVK.h"
//=============================================================================
ShaderVK::ShaderVK(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines)
{
	auto vertex_shader_spirv = CompileGlslToSpirv(ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = CompileGlslToSpirv(ShaderStage::Fragment, fragment_code, defines);

	std::tie(mPipelineLayout, mDescriptorSetLayout, mRequiredDescriptorBindings) = CreatePipelineLayout({
		vertex_shader_spirv, fragment_shader_spirv });

	auto vertex_shader_module_create_info = vk::ShaderModuleCreateInfo()
		.setCode(vertex_shader_spirv);

	auto fragment_shader_module_create_info = vk::ShaderModuleCreateInfo()
		.setCode(fragment_shader_spirv);

	mVertexShaderModule = gContext.device.createShaderModule(vertex_shader_module_create_info);
	mFragmentShaderModule = gContext.device.createShaderModule(fragment_shader_module_create_info);
}
//=============================================================================
RaytracingShaderVK::RaytracingShaderVK(const std::string& raygen_code, const std::vector<std::string>& miss_codes, const std::string& closesthit_code, std::vector<std::string> defines)
{
	auto raygen_shader_spirv = CompileGlslToSpirv(ShaderStage::Raygen, raygen_code);
	auto closesthit_shader_spirv = CompileGlslToSpirv(ShaderStage::ClosestHit, closesthit_code);

	auto raygen_shader_module_create_info = vk::ShaderModuleCreateInfo()
		.setCode(raygen_shader_spirv);

	auto closesthit_shader_module_create_info = vk::ShaderModuleCreateInfo()
		.setCode(closesthit_shader_spirv);

	mRaygenShaderModule = gContext.device.createShaderModule(raygen_shader_module_create_info);
	mClosestHitShaderModule = gContext.device.createShaderModule(closesthit_shader_module_create_info);

	std::vector<std::vector<uint32_t>> spirvs = {
		raygen_shader_spirv,
		closesthit_shader_spirv
	};

	for (const auto& miss_code : miss_codes)
	{
		auto miss_shader_spirv = CompileGlslToSpirv(ShaderStage::Miss, miss_code);

		auto miss_shader_module_create_info = vk::ShaderModuleCreateInfo()
			.setCode(miss_shader_spirv);

		auto miss_shader_module = gContext.device.createShaderModule(miss_shader_module_create_info);

		mMissShaderModules.push_back(std::move(miss_shader_module));
		spirvs.push_back(miss_shader_spirv);
	}

	std::tie(mPipelineLayout, mDescriptorSetLayout, mRequiredDescriptorBindings) = CreatePipelineLayout(spirvs);
}
//=============================================================================
#endif // RENDER_VULKAN