#pragma once

#include "RHICore.h"

std::vector<uint32_t> CompileGlslToSpirv(ShaderStage stage, const std::string& code, const std::vector<std::string>& defines = {});
std::string CompileSpirvToHlsl(const std::vector<uint32_t>& spirv, uint32_t version);
std::string CompileSpirvToGlsl(const std::vector<uint32_t>& spirv, bool es = false, uint32_t version = 450, bool enable420packExtension = true, bool forceFlattenedIOBlocks = false);

struct ShaderReflection final
{
	enum class DescriptorType
	{
		CombinedImageSampler,
		UniformBuffer,
		StorageImage,
		AccelerationStructure,
		StorageBuffer
	};

	struct Descriptor
	{
		std::string name;
		std::string type_name;
	};

	std::unordered_map<DescriptorType, std::unordered_map<uint32_t, Descriptor>>  typedDescriptorBindings;
	std::unordered_map<uint32_t/*set*/, std::unordered_set<uint32_t>/*bindings*/> descriptorSets;
	ShaderStage stage;
};

ShaderReflection MakeSpirvReflection(const std::vector<uint32_t>& spirv);