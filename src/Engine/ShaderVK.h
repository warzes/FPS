#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"

class ShaderVK : public ObjectVK
{
public:
	ShaderVK(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines);

	const auto& GetPipelineLayout() const { return mPipelineLayout; }
	const auto& GetVertexShaderModule() const { return mVertexShaderModule; }
	const auto& GetFragmentShaderModule() const { return mFragmentShaderModule; }
	const auto& GetRequiredDescriptorBindings() const { return mRequiredDescriptorBindings; }

private:
	vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
	vk::raii::PipelineLayout mPipelineLayout = nullptr;
	vk::raii::ShaderModule mVertexShaderModule = nullptr;
	vk::raii::ShaderModule mFragmentShaderModule = nullptr;
	std::vector<vk::DescriptorSetLayoutBinding> mRequiredDescriptorBindings;
};

class RaytracingShaderVK : public ObjectVK
{
public:
	RaytracingShaderVK(const std::string& raygen_code, const std::vector<std::string>& miss_codes, const std::string& closesthit_code, std::vector<std::string> defines);

	const auto& GetRaygenShaderModule() const { return mRaygenShaderModule; }
	const auto& GetMissShaderModules() const { return mMissShaderModules; }
	const auto& GetClosestHitShaderModule() const { return mClosestHitShaderModule; }
	const auto& GetPipelineLayout() const { return mPipelineLayout; }
	const auto& GetRequiredDescriptorBindings() const { return mRequiredDescriptorBindings; }

private:
	vk::raii::ShaderModule mRaygenShaderModule = nullptr;
	std::vector<vk::raii::ShaderModule> mMissShaderModules;
	vk::raii::ShaderModule mClosestHitShaderModule = nullptr;
	vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
	vk::raii::PipelineLayout mPipelineLayout = nullptr;
	std::vector<vk::DescriptorSetLayoutBinding> mRequiredDescriptorBindings;
};

#endif // RENDER_VULKAN