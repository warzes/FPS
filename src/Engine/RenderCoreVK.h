#pragma once

#if RENDER_VULKAN

#include "RHICore.h"
#include "ShaderCompiler.h"

class ObjectVK;
class ShaderVK;
class RaytracingShaderVK;
class UniformBufferVK;
class StorageBufferVK;
class BottomLevelAccelerationStructureVK;
class TopLevelAccelerationStructureVK;
class TextureVK;
class RenderTargetVK;
class VertexBufferVK;
class IndexBufferVK;

using VulkanObject = std::variant<
	vk::raii::Buffer,
	vk::raii::Image,
	vk::raii::DeviceMemory,
	vk::raii::Pipeline,
	vk::raii::AccelerationStructureKHR
>;

static const std::unordered_map<VertexFormat, vk::Format> VertexFormatMap = {
	{ VertexFormat::Float1, vk::Format::eR32Sfloat },
	{ VertexFormat::Float2, vk::Format::eR32G32Sfloat },
	{ VertexFormat::Float3, vk::Format::eR32G32B32Sfloat },
	{ VertexFormat::Float4, vk::Format::eR32G32B32A32Sfloat },
	{ VertexFormat::UChar1Normalized, vk::Format::eR8Unorm },
	{ VertexFormat::UChar2Normalized, vk::Format::eR8G8Unorm },
	{ VertexFormat::UChar4Normalized, vk::Format::eR8G8B8A8Unorm },
	{ VertexFormat::UChar1, vk::Format::eR8Uint },
	{ VertexFormat::UChar2, vk::Format::eR8G8Uint },
	{ VertexFormat::UChar4, vk::Format::eR8G8B8A8Uint }
};

static const std::unordered_map<PixelFormat, vk::Format> PixelFormatMap = {
	{ PixelFormat::R32Float, vk::Format::eR32Sfloat },
	{ PixelFormat::RG32Float, vk::Format::eR32G32Sfloat },
	{ PixelFormat::RGB32Float, vk::Format::eR32G32B32Sfloat },
	{ PixelFormat::RGBA32Float, vk::Format::eR32G32B32A32Sfloat },
	{ PixelFormat::R8UNorm, vk::Format::eR8Unorm },
	{ PixelFormat::RG8UNorm, vk::Format::eR8G8Unorm },
	{ PixelFormat::RGBA8UNorm, vk::Format::eR8G8B8A8Unorm },
};

static const std::unordered_map<vk::Format, PixelFormat> ReversedPixelFormatMap = {
	{ vk::Format::eR32Sfloat, PixelFormat::R32Float },
	{ vk::Format::eR32G32Sfloat, PixelFormat::RG32Float },
	{ vk::Format::eR32G32B32Sfloat, PixelFormat::RGB32Float },
	{ vk::Format::eR32G32B32A32Sfloat, PixelFormat::RGBA32Float },
	{ vk::Format::eR8Unorm, PixelFormat::R8UNorm },
	{ vk::Format::eR8G8Unorm, PixelFormat::RG8UNorm },
	{ vk::Format::eR8G8B8A8Unorm, PixelFormat::RGBA8UNorm }
};

const static std::unordered_map<ComparisonFunction, vk::CompareOp> CompareOpMap = {
	{ ComparisonFunction::Always, vk::CompareOp::eAlways },
	{ ComparisonFunction::Never, vk::CompareOp::eNever },
	{ ComparisonFunction::Less, vk::CompareOp::eLess },
	{ ComparisonFunction::Equal, vk::CompareOp::eEqual },
	{ ComparisonFunction::NotEqual, vk::CompareOp::eNotEqual },
	{ ComparisonFunction::LessEqual, vk::CompareOp::eLessOrEqual },
	{ ComparisonFunction::Greater, vk::CompareOp::eGreater },
	{ ComparisonFunction::GreaterEqual, vk::CompareOp::eGreaterOrEqual }
};

const static std::unordered_map<ShaderStage, vk::ShaderStageFlagBits> ShaderStageMap = {
	{ ShaderStage::Vertex, vk::ShaderStageFlagBits::eVertex },
	{ ShaderStage::Fragment, vk::ShaderStageFlagBits::eFragment },
	{ ShaderStage::Raygen, vk::ShaderStageFlagBits::eRaygenKHR },
	{ ShaderStage::Miss, vk::ShaderStageFlagBits::eMissKHR },
	{ ShaderStage::ClosestHit, vk::ShaderStageFlagBits::eClosestHitKHR }
};

const static std::unordered_map<ShaderReflection::DescriptorType, vk::DescriptorType> ShaderTypeMap = {
	{ ShaderReflection::DescriptorType::CombinedImageSampler, vk::DescriptorType::eCombinedImageSampler },
	{ ShaderReflection::DescriptorType::UniformBuffer, vk::DescriptorType::eUniformBuffer },
	{ ShaderReflection::DescriptorType::StorageImage, vk::DescriptorType::eStorageImage },
	{ ShaderReflection::DescriptorType::AccelerationStructure, vk::DescriptorType::eAccelerationStructureKHR },
	{ ShaderReflection::DescriptorType::StorageBuffer, vk::DescriptorType::eStorageBuffer }
};

constexpr const vk::Format DefaultDepthStencilFormat = vk::Format::eD32SfloatS8Uint;

inline vk::IndexType GetIndexTypeFromStride(size_t stride)
{
	return stride == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
}

class ObjectVK
{
public:
	virtual ~ObjectVK() {}
};

struct RaytracingShaderBindingTable
{
	vk::raii::Buffer raygen_buffer;
	vk::raii::DeviceMemory raygen_memory;
	vk::StridedDeviceAddressRegionKHR raygen_address;

	vk::raii::Buffer miss_buffer;
	vk::raii::DeviceMemory miss_memory;
	vk::StridedDeviceAddressRegionKHR miss_address;

	vk::raii::Buffer hit_buffer;
	vk::raii::DeviceMemory hit_memory;
	vk::StridedDeviceAddressRegionKHR hit_address;

	vk::StridedDeviceAddressRegionKHR callable_address;
};

#endif // RENDER_VULKAN