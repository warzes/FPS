#include "stdafx.h"
#if RENDER_VULKAN
#include "ContextVK.h"
#include "RenderTargetVK.h"
#include "BufferVK.h"
#include "AccelerationStructureVK.h"
#include "ShaderVK.h"
//=============================================================================
void RenderContext::Clear()
{
	for (auto object : objects)
		delete object;
}
//=============================================================================
uint32_t RenderContext::GetBackBufferWidth()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetWidth() : width;
}
//=============================================================================
uint32_t RenderContext::GetBackBufferHeight()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetHeight() : height;
}
//=============================================================================
vk::Format RenderContext::GetBackbufferFormat()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetFormat() : PixelFormatMap.at(PixelFormat::RGBA8UNorm); //gContext.surfaceFormat.format;
}
//=============================================================================
uint32_t GetMemoryType(vk::MemoryPropertyFlags properties, uint32_t type_bits)
{
	auto prop = gContext.physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
		if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
			return i;

	return 0xFFFFFFFF; // Unable to find memoryType
}
//=============================================================================
std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(uint64_t size, vk::BufferUsageFlags usage)
{
	auto buffer_create_info = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);

	auto buffer = gContext.device.createBuffer(buffer_create_info);

	auto memory_requirements = buffer.getMemoryRequirements();
	auto memory_type = GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, memory_requirements.memoryTypeBits);

	auto memory_allocate_info = vk::MemoryAllocateInfo()
		.setAllocationSize(memory_requirements.size)
		.setMemoryTypeIndex(memory_type);

	auto device_memory = gContext.device.allocateMemory(memory_allocate_info);

	buffer.bindMemory(*device_memory, 0);

	return { std::move(buffer), std::move(device_memory) };
}
//=============================================================================
vk::raii::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels)
{
	auto image_subresource_range = vk::ImageSubresourceRange()
		.setAspectMask(aspect_flags)
		.setLevelCount(mip_levels)
		.setLayerCount(1);

	auto image_view_create_info = vk::ImageViewCreateInfo()
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setSubresourceRange(image_subresource_range);

	return gContext.device.createImageView(image_view_create_info);
}
//=============================================================================
std::tuple<vk::raii::Image, vk::raii::DeviceMemory, vk::raii::ImageView> CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels)
{
	auto image_create_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent({ width, height, 1 })
		.setMipLevels(mip_levels)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setInitialLayout(vk::ImageLayout::eUndefined);

	auto image = gContext.device.createImage(image_create_info);

	auto memory_requirements = image.getMemoryRequirements();
	auto memory_type = GetMemoryType(vk::MemoryPropertyFlagBits::eDeviceLocal, memory_requirements.memoryTypeBits);

	auto memory_allocate_info = vk::MemoryAllocateInfo()
		.setAllocationSize(memory_requirements.size)
		.setMemoryTypeIndex(memory_type);

	auto device_memory = gContext.device.allocateMemory(memory_allocate_info);

	image.bindMemory(*device_memory, 0);

	auto image_view = CreateImageView(*image, format, aspect_flags, mip_levels);

	return { std::move(image), std::move(device_memory), std::move(image_view) };
}
//=============================================================================
vk::DeviceAddress GetBufferDeviceAddress(vk::Buffer buffer)
{
	auto info = vk::BufferDeviceAddressInfo()
		.setBuffer(buffer);

	return gContext.device.getBufferAddress(info);
};
//=============================================================================
void WriteToBuffer(vk::raii::DeviceMemory& memory, const void* data, size_t size)
{
	auto ptr = memory.mapMemory(0, size);
	memcpy(ptr, data, size);
	memory.unmapMemory();
};
//=============================================================================
void DestroyStaging(VulkanObject&& object)
{
	gContext.getCurrentFrame().staging_objects.push_back(std::move(object));
}
//=============================================================================
void ReleaseStaging()
{
	gContext.getCurrentFrame().staging_objects.clear();
}
//=============================================================================
std::tuple<vk::raii::PipelineLayout, vk::raii::DescriptorSetLayout, std::vector<vk::DescriptorSetLayoutBinding>> CreatePipelineLayout(const std::vector<std::vector<uint32_t>>& spirvs)
{
	std::vector<vk::DescriptorSetLayoutBinding> required_descriptor_bindings;

	for (const auto& spirv : spirvs)
	{
		auto reflection = MakeSpirvReflection(spirv);

		for (const auto& [type, descriptor_bindings] : reflection.typedDescriptorBindings)
		{
			for (const auto& [binding, descriptor] : descriptor_bindings)
			{
				bool overwritten = false;

				for (auto& _binding : required_descriptor_bindings)
				{
					if (_binding.binding != binding)
						continue;

					_binding.stageFlags |= ShaderStageMap.at(reflection.stage);
					overwritten = true;
					break;
				}

				if (overwritten)
					continue;

				auto descriptor_set_layout_binding = vk::DescriptorSetLayoutBinding()
					.setDescriptorType(ShaderTypeMap.at(type))
					.setDescriptorCount(1)
					.setBinding(binding)
					.setStageFlags(ShaderStageMap.at(reflection.stage));

				required_descriptor_bindings.push_back(descriptor_set_layout_binding);
			}
		}
	}
	auto descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo()
		.setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR)
		.setBindings(required_descriptor_bindings);

	auto descriptor_set_layout = gContext.device.createDescriptorSetLayout(descriptor_set_layout_create_info);

	auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo()
		.setSetLayouts(*descriptor_set_layout);

	auto pipeline_layout = gContext.device.createPipelineLayout(pipeline_layout_create_info);

	return { std::move(pipeline_layout), std::move(descriptor_set_layout), required_descriptor_bindings };
}
//=============================================================================
void BeginRenderPass()
{
	assert(!gContext.render_pass_active);
	gContext.render_pass_active = true;

	auto targets = gContext.render_targets;

	if (targets.empty())
		targets = { gContext.getCurrentFrame().swapchain_target.get() };

	std::vector<vk::RenderingAttachmentInfo> color_attachments;
	std::optional<vk::RenderingAttachmentInfo> depth_stencil_attachment;

	for (auto target : targets)
	{
		auto color_attachment = vk::RenderingAttachmentInfo()
			.setImageView(*target->GetTexture()->GetImageView())
			.setImageLayout(vk::ImageLayout::eGeneral)
			.setLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore);

		color_attachments.push_back(color_attachment);

		if (!depth_stencil_attachment.has_value())
		{
			depth_stencil_attachment = vk::RenderingAttachmentInfo()
				.setImageView(*target->GetDepthStencilView())
				.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.setLoadOp(vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore);
		}
	}

	auto width = gContext.GetBackBufferWidth();
	auto height = gContext.GetBackBufferHeight();

	auto rendering_info = vk::RenderingInfo()
		.setRenderArea({ { 0, 0 }, { width, height } })
		.setLayerCount(1)
		.setColorAttachments(color_attachments);

	if (depth_stencil_attachment.has_value())
	{
		rendering_info.setPDepthAttachment(&depth_stencil_attachment.value());
		rendering_info.setPStencilAttachment(&depth_stencil_attachment.value());
	}

	gContext.getCurrentFrame().command_buffer.beginRendering(rendering_info);
}
//=============================================================================
void EndRenderPass()
{
	assert(gContext.render_pass_active);
	gContext.render_pass_active = false;

	gContext.getCurrentFrame().command_buffer.endRendering();
}
//=============================================================================
void EnsureRenderPassActivated()
{
	if (gContext.render_pass_active)
		return;

	BeginRenderPass();
}
//=============================================================================
void EnsureRenderPassDeactivated()
{
	if (!gContext.render_pass_active)
		return;

	EndRenderPass();
}
//=============================================================================
void SetImageMemoryBarrier(const vk::raii::CommandBuffer& cmdbuf, vk::Image image, vk::ImageAspectFlags aspect_mask, vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t base_mip_level, uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count)
{
	assert(new_layout != vk::ImageLayout::eUndefined && new_layout != vk::ImageLayout::ePreinitialized);

	constexpr vk::PipelineStageFlags depth_stage_mask = vk::PipelineStageFlagBits::eEarlyFragmentTests |
		vk::PipelineStageFlagBits::eLateFragmentTests;

	constexpr vk::PipelineStageFlags sampled_stage_mask = vk::PipelineStageFlagBits::eVertexShader |
		vk::PipelineStageFlagBits::eFragmentShader;

	vk::PipelineStageFlags src_stage_mask = vk::PipelineStageFlagBits::eTopOfPipe;
	vk::PipelineStageFlags dst_stage_mask = vk::PipelineStageFlagBits::eBottomOfPipe;

	vk::AccessFlags src_access_mask;
	vk::AccessFlags dst_access_mask;

	switch (old_layout)
	{
	case vk::ImageLayout::eUndefined:
		break;

	case vk::ImageLayout::eGeneral:
		src_stage_mask = vk::PipelineStageFlagBits::eAllCommands;
		src_access_mask = vk::AccessFlagBits::eMemoryWrite;
		break;

	case vk::ImageLayout::eColorAttachmentOptimal:
		src_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		src_access_mask = vk::AccessFlagBits::eColorAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		src_stage_mask = depth_stage_mask;
		src_access_mask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
		src_stage_mask = depth_stage_mask | sampled_stage_mask;
		break;

	case vk::ImageLayout::eShaderReadOnlyOptimal:
		src_stage_mask = sampled_stage_mask;
		break;

	case vk::ImageLayout::eTransferSrcOptimal:
		src_stage_mask = vk::PipelineStageFlagBits::eTransfer;
		break;

	case vk::ImageLayout::eTransferDstOptimal:
		src_stage_mask = vk::PipelineStageFlagBits::eTransfer;
		src_access_mask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::ePreinitialized:
		src_stage_mask = vk::PipelineStageFlagBits::eHost;
		src_access_mask = vk::AccessFlagBits::eHostWrite;
		break;

	case vk::ImageLayout::ePresentSrcKHR:
		break;

	default:
		assert(false);
		break;
	}

	switch (new_layout)
	{
	case vk::ImageLayout::eGeneral:
		dst_stage_mask = vk::PipelineStageFlagBits::eAllCommands;
		dst_access_mask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite;
		break;

	case vk::ImageLayout::eColorAttachmentOptimal:
		dst_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dst_access_mask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		dst_stage_mask = depth_stage_mask;
		dst_access_mask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
		dst_stage_mask = depth_stage_mask | sampled_stage_mask;
		dst_access_mask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eShaderRead |
			vk::AccessFlagBits::eInputAttachmentRead;
		break;

	case vk::ImageLayout::eShaderReadOnlyOptimal:
		dst_stage_mask = sampled_stage_mask;
		dst_access_mask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
		break;

	case vk::ImageLayout::eTransferSrcOptimal:
		dst_stage_mask = vk::PipelineStageFlagBits::eTransfer;
		dst_access_mask = vk::AccessFlagBits::eTransferRead;
		break;

	case vk::ImageLayout::eTransferDstOptimal:
		dst_stage_mask = vk::PipelineStageFlagBits::eTransfer;
		dst_access_mask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::ePresentSrcKHR:
		// vkQueuePresentKHR performs automatic visibility operations
		break;

	default:
		assert(false);
		break;
	}

	auto subresource_range = vk::ImageSubresourceRange()
		.setAspectMask(aspect_mask)
		.setBaseMipLevel(base_mip_level)
		.setLevelCount(level_count)
		.setBaseArrayLayer(base_array_layer)
		.setLayerCount(layer_count);

	auto image_memory_barrier = vk::ImageMemoryBarrier()
		.setSrcAccessMask(src_access_mask)
		.setDstAccessMask(dst_access_mask)
		.setOldLayout(old_layout)
		.setNewLayout(new_layout)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setImage(image)
		.setSubresourceRange(subresource_range);

	cmdbuf.pipelineBarrier(src_stage_mask, dst_stage_mask, {}, {}, {}, { image_memory_barrier });
}
//=============================================================================
void SetImageMemoryBarrier(const vk::raii::CommandBuffer& cmdbuf, vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t base_mip_level, uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count)
{
	vk::ImageAspectFlags aspect_mask;
	if (new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		aspect_mask = vk::ImageAspectFlagBits::eDepth;
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
		{
			aspect_mask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		aspect_mask = vk::ImageAspectFlagBits::eColor;
	}
	SetImageMemoryBarrier(cmdbuf, image, aspect_mask, old_layout, new_layout, base_mip_level, level_count,
		base_array_layer, layer_count);
}
//=============================================================================
void SetMemoryBarrier(const vk::raii::CommandBuffer& cmdbuf, vk::PipelineStageFlags2 src_stage, vk::PipelineStageFlags2 dst_stage)
{
	auto memory_barrier = vk::MemoryBarrier2()
		.setSrcStageMask(src_stage)
		.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
		.setDstStageMask(dst_stage)
		.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

	auto dependency_info = vk::DependencyInfo()
		.setMemoryBarriers(memory_barrier);

	cmdbuf.pipelineBarrier2(dependency_info);
}
//=============================================================================
void EnsureMemoryState(const vk::raii::CommandBuffer& cmdbuf, vk::PipelineStageFlags2 stage)
{
	if (gContext.current_memory_stage == stage)
		return;

	EnsureRenderPassDeactivated();
	SetMemoryBarrier(cmdbuf, gContext.current_memory_stage, stage);

	gContext.current_memory_stage = stage;
}
//=============================================================================
void OneTimeSubmit(std::function<void(const vk::raii::CommandBuffer&)> func)
{
	auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(*gContext.commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	auto command_buffers = gContext.device.allocateCommandBuffers(command_buffer_allocate_info);
	auto cmdbuf = std::move(command_buffers.at(0));

	auto command_buffer_begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	cmdbuf.begin(command_buffer_begin_info);
	func(cmdbuf);
	cmdbuf.end();

	auto submit_info = vk::SubmitInfo()
		.setCommandBuffers(*cmdbuf);

	gContext.queue.submit(submit_info);
	gContext.queue.waitIdle();
}
//=============================================================================
void PushDescriptorBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding, vk::DescriptorType type,
	const vk::raii::Buffer& buffer)
{
	auto descriptor_buffer_info = vk::DescriptorBufferInfo()
		.setBuffer(*buffer)
		.setRange(VK_WHOLE_SIZE);

	auto write_descriptor_set = vk::WriteDescriptorSet()
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(type)
		.setBufferInfo(descriptor_buffer_info);

	cmdlist.pushDescriptorSetKHR(pipeline_bind_point,
		*pipeline_layout, 0, write_descriptor_set);
}
//=============================================================================
void PushDescriptorTexture(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point, const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)
{
	if (!gContext.sampler_states.contains(gContext.sampler_state))
	{
		auto sampler = CreateSamplerState(gContext.sampler_state);
		gContext.sampler_states.insert({ gContext.sampler_state, std::move(sampler) });
	}

	auto texture = gContext.textures.at(binding);
	texture->EnsureState(cmdlist, vk::ImageLayout::eGeneral);

	const auto& sampler = gContext.sampler_states.at(gContext.sampler_state);

	auto descriptor_image_info = vk::DescriptorImageInfo()
		.setSampler(*sampler)
		.setImageView(*texture->GetImageView())
		.setImageLayout(vk::ImageLayout::eGeneral);

	auto write_descriptor_set = vk::WriteDescriptorSet()
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setImageInfo(descriptor_image_info);

	cmdlist.pushDescriptorSetKHR(pipeline_bind_point,
		*pipeline_layout, 0, write_descriptor_set);
}
//=============================================================================
void PushDescriptorUniformBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)
{
	auto buffer = gContext.uniform_buffers.at(binding);

	PushDescriptorBuffer(cmdlist, pipeline_bind_point, pipeline_layout, binding,
		vk::DescriptorType::eUniformBuffer, buffer->GetBuffer());
}
//=============================================================================
void PushDescriptorAccelerationStructure(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)
{
	auto acceleration_structure = gContext.top_level_acceleration_structures.at(binding);

	auto write_descriptor_set_acceleration_structure = vk::WriteDescriptorSetAccelerationStructureKHR()
		.setAccelerationStructures(*acceleration_structure->GetTlas());

	auto write_descriptor_set = vk::WriteDescriptorSet()
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR)
		.setPNext(&write_descriptor_set_acceleration_structure);

	cmdlist.pushDescriptorSetKHR(pipeline_bind_point,
		*pipeline_layout, 0, write_descriptor_set);
}
//=============================================================================
void PushDescriptorStorageImage(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)
{
	auto texture = gContext.render_targets.at(0)->GetTexture();
	texture->EnsureState(cmdlist, vk::ImageLayout::eGeneral);

	auto descriptor_image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eGeneral)
		.setImageView(*texture->GetImageView());

	auto write_descriptor_set = vk::WriteDescriptorSet()
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eStorageImage)
		.setImageInfo(descriptor_image_info);

	cmdlist.pushDescriptorSetKHR(pipeline_bind_point,
		*pipeline_layout, 0, write_descriptor_set);
}
//=============================================================================
void PushDescriptorStorageBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)
{
	auto buffer = gContext.storage_buffers.at(binding);

	PushDescriptorBuffer(cmdlist, pipeline_bind_point, pipeline_layout, binding,
		vk::DescriptorType::eStorageBuffer, buffer->GetBuffer());
}
//=============================================================================
void PushDescriptors(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, const std::vector<vk::DescriptorSetLayoutBinding>& required_descriptor_bindings,
	const std::unordered_set<uint32_t>& ignore_bindings)
{
	using PushTypedCallback = std::function<void(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
		const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding)>;

	static const std::unordered_map<vk::DescriptorType, PushTypedCallback> PushTypedCallbacks = {
		{ vk::DescriptorType::eCombinedImageSampler, PushDescriptorTexture },
		{ vk::DescriptorType::eUniformBuffer, PushDescriptorUniformBuffer },
		{ vk::DescriptorType::eAccelerationStructureKHR, PushDescriptorAccelerationStructure },
		{ vk::DescriptorType::eStorageImage, PushDescriptorStorageImage },
		{ vk::DescriptorType::eStorageBuffer, PushDescriptorStorageBuffer },
	};

	for (const auto& required_descriptor_binding : required_descriptor_bindings)
	{
		auto binding = required_descriptor_binding.binding;

		if (ignore_bindings.contains(binding))
			continue;

		auto type = required_descriptor_binding.descriptorType;
		const auto& callback = PushTypedCallbacks.at(type);
		callback(cmdlist, pipeline_bind_point, pipeline_layout, binding);
	}
}
//=============================================================================
RaytracingShaderBindingTable CreateRaytracingShaderBindingTable(const vk::raii::Pipeline& pipeline)
{
	auto ray_tracing_pipeline_properties = gContext.physicalDevice.getProperties2<vk::PhysicalDeviceProperties2,
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>().get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

	auto handle_size = ray_tracing_pipeline_properties.shaderGroupHandleSize;
	auto handle_size_aligned = AlignUp(handle_size, ray_tracing_pipeline_properties.shaderGroupHandleAlignment);

	auto [raygen_buffer, raygen_memory] = CreateBuffer(handle_size_aligned,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto [miss_buffer, miss_memory] = CreateBuffer(handle_size_aligned,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	auto [hit_buffer, hit_memory] = CreateBuffer(handle_size_aligned,
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	const auto& shader = gContext.raytracing_pipeline_state.shader;

	auto raygen_shader_count = 1;
	auto miss_shader_count = shader->GetMissShaderModules().size();
	auto hit_shader_count = 1;

	auto group_count = raygen_shader_count + miss_shader_count + hit_shader_count;
	auto sbt_size = group_count * handle_size_aligned;

	auto shader_handle_storage = pipeline.getRayTracingShaderGroupHandlesKHR<uint8_t>(0, (uint32_t)group_count, sbt_size);

	auto get_shader_ptr = [&](size_t index) {
		return shader_handle_storage.data() + (handle_size_aligned * index);
		};

	WriteToBuffer(raygen_memory, get_shader_ptr(0), handle_size * raygen_shader_count);
	WriteToBuffer(miss_memory, get_shader_ptr(raygen_shader_count), handle_size * miss_shader_count);
	WriteToBuffer(hit_memory, get_shader_ptr(raygen_shader_count + miss_shader_count), handle_size * hit_shader_count);

	auto raygen_address = vk::StridedDeviceAddressRegionKHR()
		.setStride(handle_size_aligned)
		.setSize(handle_size_aligned)
		.setDeviceAddress(GetBufferDeviceAddress(*raygen_buffer));

	auto miss_address = vk::StridedDeviceAddressRegionKHR()
		.setStride(handle_size_aligned)
		.setSize(handle_size_aligned)
		.setDeviceAddress(GetBufferDeviceAddress(*miss_buffer));

	auto hit_address = vk::StridedDeviceAddressRegionKHR()
		.setStride(handle_size_aligned)
		.setSize(handle_size_aligned)
		.setDeviceAddress(GetBufferDeviceAddress(*hit_buffer));

	auto callable_shader_binding_table = vk::StridedDeviceAddressRegionKHR();

	return RaytracingShaderBindingTable{
		.raygen_buffer = std::move(raygen_buffer),
		.raygen_memory = std::move(raygen_memory),
		.raygen_address = raygen_address,

		.miss_buffer = std::move(miss_buffer),
		.miss_memory = std::move(miss_memory),
		.miss_address = miss_address,

		.hit_buffer = std::move(hit_buffer),
		.hit_memory = std::move(hit_memory),
		.hit_address = hit_address,

		.callable_address = callable_shader_binding_table
	};
}
//=============================================================================
vk::raii::Pipeline CreateGraphicsPipeline(const PipelineStateVK& pipeline_state)
{
	auto pipeline_shader_stage_create_info = {
		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(*pipeline_state.shader->GetVertexShaderModule())
			.setPName("main"),

		vk::PipelineShaderStageCreateInfo()
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(*pipeline_state.shader->GetFragmentShaderModule())
			.setPName("main")
	};

	auto pipeline_input_assembly_state_create_info = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	auto pipeline_viewport_state_create_info = vk::PipelineViewportStateCreateInfo()
		.setViewportCount(1)
		.setScissorCount(1);

	auto pipeline_rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill);

	auto pipeline_multisample_state_create_info = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	auto pipeline_depth_stencil_state_create_info = vk::PipelineDepthStencilStateCreateInfo();

	auto pipeline_color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo()
		.setAttachmentCount((uint32_t)pipeline_state.color_attachment_formats.size());

	std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions;
	std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;

	for (size_t i = 0; i < pipeline_state.input_layouts.size(); i++)
	{
		const auto& input_layout = pipeline_state.input_layouts.at(i);

		static const std::unordered_map<InputLayout::Rate, vk::VertexInputRate> InputRateMap = {
			{ InputLayout::Rate::Vertex, vk::VertexInputRate::eVertex },
			{ InputLayout::Rate::Instance, vk::VertexInputRate::eInstance },
		};

		auto vertex_input_binding_description = vk::VertexInputBindingDescription()
			.setInputRate(InputRateMap.at(input_layout.rate))
			.setBinding((uint32_t)i);

		vertex_input_binding_descriptions.push_back(vertex_input_binding_description);

		for (const auto& [location, attribute] : input_layout.attributes)
		{
			auto vertex_input_attribute_description = vk::VertexInputAttributeDescription()
				.setBinding((uint32_t)i)
				.setLocation(location)
				.setFormat(VertexFormatMap.at(attribute.format))
				.setOffset((uint32_t)attribute.offset);

			vertex_input_attribute_descriptions.push_back(vertex_input_attribute_description);
		}
	}

	// TODO: use dynamic state for InputLayout
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_vertex_input_dynamic_state.html

	auto pipeline_vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(vertex_input_binding_descriptions)
		.setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

	auto dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::ePrimitiveTopology,
		vk::DynamicState::eLineWidth,
		vk::DynamicState::eCullMode,
		vk::DynamicState::eFrontFace,
		vk::DynamicState::eVertexInputBindingStride,
		vk::DynamicState::eDepthTestEnable,
		vk::DynamicState::eDepthCompareOp,
		vk::DynamicState::eDepthWriteEnable,
		vk::DynamicState::eColorWriteMaskEXT,
		vk::DynamicState::eColorBlendEquationEXT,
		vk::DynamicState::eColorBlendEnableEXT,
		vk::DynamicState::eStencilTestEnable
	};

	auto pipeline_dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo()
		.setDynamicStates(dynamic_states);

	auto pipeline_rendering_create_info = vk::PipelineRenderingCreateInfo()
		.setColorAttachmentFormats(pipeline_state.color_attachment_formats)
		.setDepthAttachmentFormat(pipeline_state.depth_stencil_format.value())
		.setStencilAttachmentFormat(pipeline_state.depth_stencil_format.value());

	auto graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo()
		.setLayout(*pipeline_state.shader->GetPipelineLayout())
		.setFlags(vk::PipelineCreateFlagBits())
		.setStages(pipeline_shader_stage_create_info)
		.setPVertexInputState(&pipeline_vertex_input_state_create_info)
		.setPInputAssemblyState(&pipeline_input_assembly_state_create_info)
		.setPViewportState(&pipeline_viewport_state_create_info)
		.setPRasterizationState(&pipeline_rasterization_state_create_info)
		.setPMultisampleState(&pipeline_multisample_state_create_info)
		.setPDepthStencilState(&pipeline_depth_stencil_state_create_info)
		.setPColorBlendState(&pipeline_color_blend_state_create_info) // TODO: this can be nullptr https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGraphicsPipelineCreateInfo.html
		.setPDynamicState(&pipeline_dynamic_state_create_info)
		.setRenderPass(nullptr)
		.setPNext(&pipeline_rendering_create_info);

	return gContext.device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info);
}
//=============================================================================
vk::raii::Pipeline CreateRaytracingPipeline(const RaytracingPipelineStateVK& pipeline_state)
{
	std::vector<vk::PipelineShaderStageCreateInfo> stages;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> groups;

	auto addShader = [&](vk::ShaderStageFlagBits stage, vk::RayTracingShaderGroupTypeKHR group_type, const vk::raii::ShaderModule& shader_module) {
		auto stage_create_info = vk::PipelineShaderStageCreateInfo()
			.setStage(stage)
			.setModule(*shader_module)
			.setPName("main");

		stages.push_back(stage_create_info);

		auto group = vk::RayTracingShaderGroupCreateInfoKHR()
			.setType(group_type)
			.setGeneralShader(VK_SHADER_UNUSED_KHR)
			.setClosestHitShader(VK_SHADER_UNUSED_KHR)
			.setAnyHitShader(VK_SHADER_UNUSED_KHR)
			.setIntersectionShader(VK_SHADER_UNUSED_KHR);

		auto index = (uint32_t)(stages.size() - 1);

		if (group_type == vk::RayTracingShaderGroupTypeKHR::eGeneral)
		{
			group.setGeneralShader(index);
		}
		else if (group_type == vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup && stage == vk::ShaderStageFlagBits::eClosestHitKHR)
		{
			group.setClosestHitShader(index);
		}
		else
		{
			assert(false);
		}

		groups.push_back(group);
		};

	addShader(vk::ShaderStageFlagBits::eRaygenKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral,
		pipeline_state.shader->GetRaygenShaderModule());

	for (const auto& miss_module : pipeline_state.shader->GetMissShaderModules())
	{
		addShader(vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral, miss_module);
	}

	addShader(vk::ShaderStageFlagBits::eClosestHitKHR, vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,
		pipeline_state.shader->GetClosestHitShaderModule());

	auto raytracing_pipeline_create_info = vk::RayTracingPipelineCreateInfoKHR()
		.setLayout(*pipeline_state.shader->GetPipelineLayout())
		.setStages(stages)
		.setGroups(groups)
		.setMaxPipelineRayRecursionDepth(3);

	return gContext.device.createRayTracingPipelineKHR(nullptr, nullptr, raytracing_pipeline_create_info);
}
//=============================================================================
void EnsureVertexBuffers(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.vertex_buffers_dirty)
		return;

	gContext.vertex_buffers_dirty = false;

	std::vector<vk::Buffer> buffers;
	std::vector<vk::DeviceSize> offsets;
	std::vector<vk::DeviceSize> strides;

	for (auto vertex_buffer : gContext.vertex_buffers)
	{
		buffers.push_back(*vertex_buffer->GetBuffer());
		offsets.push_back(0);
		strides.push_back(vertex_buffer->GetStride());
	}

	cmdlist.bindVertexBuffers2(0, buffers, offsets, nullptr, strides);
}
//=============================================================================
void EnsureIndexBuffer(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.index_buffer_dirty)
		return;

	gContext.index_buffer_dirty = false;

	auto index_type = GetIndexTypeFromStride(gContext.index_buffer->GetStride());
	cmdlist.bindIndexBuffer(*gContext.index_buffer->GetBuffer(), 0, index_type);
}
//=============================================================================
void EnsureTopology(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.topology_dirty)
		return;

	gContext.topology_dirty = false;

	static const std::unordered_map<Topology, vk::PrimitiveTopology> TopologyMap = {
		{ Topology::PointList, vk::PrimitiveTopology::ePointList },
		{ Topology::LineList, vk::PrimitiveTopology::eLineList },
		{ Topology::LineStrip, vk::PrimitiveTopology::eLineStrip },
		{ Topology::TriangleList, vk::PrimitiveTopology::eTriangleList },
		{ Topology::TriangleStrip, vk::PrimitiveTopology::eTriangleStrip },
	};

	auto topology = TopologyMap.at(gContext.topology);

	cmdlist.setPrimitiveTopology(topology);
}
//=============================================================================
void EnsureViewport(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.viewport_dirty)
		return;

	gContext.viewport_dirty = false;

	auto width = static_cast<float>(gContext.GetBackBufferWidth());
	auto height = static_cast<float>(gContext.GetBackBufferHeight());

	auto value = gContext.viewport.value_or(Viewport{ { 0.0f, 0.0f }, { width, height } });

	auto viewport = vk::Viewport()
		.setX(value.position.x)
		.setY(value.size.y - value.position.y)
		.setWidth(value.size.x)
		.setHeight(-value.size.y)
		.setMinDepth(value.minDepth)
		.setMaxDepth(value.maxDepth);

	cmdlist.setViewport(0, { viewport });
}
//=============================================================================
void EnsureScissor(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.scissor_dirty)
		return;

	gContext.scissor_dirty = false;

	auto width = static_cast<float>(gContext.GetBackBufferWidth());
	auto height = static_cast<float>(gContext.GetBackBufferHeight());

	auto value = gContext.scissor.value_or(Scissor{ { 0.0f, 0.0f }, { width, height } });

	auto rect = vk::Rect2D()
		.setOffset({ static_cast<int32_t>(value.position.x), static_cast<int32_t>(value.position.y) })
		.setExtent({ static_cast<uint32_t>(value.size.x), static_cast<uint32_t>(value.size.y) });

	if (rect.offset.x < 0)
	{
		rect.extent.width -= rect.offset.x;
		rect.offset.x = 0;
	}

	if (rect.offset.y < 0)
	{
		rect.extent.height -= rect.offset.y;
		rect.offset.y = 0;
	}

	if (rect.extent.width < 0)
		rect.extent.width = 0;

	if (rect.extent.height < 0)
		rect.extent.height = 0;

	cmdlist.setScissor(0, { rect });
}
//=============================================================================
void EnsureCullMode(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.cull_mode_dirty)
		return;

	gContext.cull_mode_dirty = false;

	const static std::unordered_map<CullMode, vk::CullModeFlags> CullModeMap = {
		{ CullMode::None, vk::CullModeFlagBits::eNone },
		{ CullMode::Front, vk::CullModeFlagBits::eFront },
		{ CullMode::Back, vk::CullModeFlagBits::eBack },
	};

	cmdlist.setCullMode(CullModeMap.at(gContext.cull_mode));
}
//=============================================================================
void EnsureFrontFace(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.front_face_dirty)
		return;

	gContext.front_face_dirty = false;

	const static std::unordered_map<FrontFace, vk::FrontFace> FrontFaceMap = {
		{ FrontFace::Clockwise, vk::FrontFace::eClockwise },
		{ FrontFace::CounterClockwise, vk::FrontFace::eCounterClockwise },
	};

	cmdlist.setFrontFace(FrontFaceMap.at(gContext.front_face));
}
//=============================================================================
void EnsureBlendMode(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.blend_mode_dirty)
		return;

	gContext.blend_mode_dirty = false;

	static const std::unordered_map<Blend, vk::BlendFactor> BlendFactorMap = {
		{ Blend::One, vk::BlendFactor::eOne },
		{ Blend::Zero, vk::BlendFactor::eZero },
		{ Blend::SrcColor, vk::BlendFactor::eSrcColor },
		{ Blend::InvSrcColor, vk::BlendFactor::eOneMinusSrcColor },
		{ Blend::SrcAlpha, vk::BlendFactor::eSrcAlpha },
		{ Blend::InvSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha },
		{ Blend::DstColor, vk::BlendFactor::eDstColor },
		{ Blend::InvDstColor, vk::BlendFactor::eOneMinusDstColor },
		{ Blend::DstAlpha, vk::BlendFactor::eDstAlpha },
		{ Blend::InvDstAlpha, vk::BlendFactor::eOneMinusDstAlpha }
	};

	static const std::unordered_map<BlendFunction, vk::BlendOp> BlendFuncMap = {
		{ BlendFunction::Add, vk::BlendOp::eAdd },
		{ BlendFunction::Subtract, vk::BlendOp::eSubtract },
		{ BlendFunction::ReverseSubtract, vk::BlendOp::eReverseSubtract },
		{ BlendFunction::Min, vk::BlendOp::eMin },
		{ BlendFunction::Max, vk::BlendOp::eMax },
	};

	const auto& blend_mode = gContext.blend_mode.value_or(BlendStates::Opaque);

	auto color_mask = vk::ColorComponentFlags();

	if (blend_mode.colorMask.red)
		color_mask |= vk::ColorComponentFlagBits::eR;

	if (blend_mode.colorMask.green)
		color_mask |= vk::ColorComponentFlagBits::eG;

	if (blend_mode.colorMask.blue)
		color_mask |= vk::ColorComponentFlagBits::eB;

	if (blend_mode.colorMask.alpha)
		color_mask |= vk::ColorComponentFlagBits::eA;

	auto color_blend_equation = vk::ColorBlendEquationEXT()
		.setSrcColorBlendFactor(BlendFactorMap.at(blend_mode.colorSrc))
		.setDstColorBlendFactor(BlendFactorMap.at(blend_mode.colorDst))
		.setColorBlendOp(BlendFuncMap.at(blend_mode.colorFunc))
		.setSrcAlphaBlendFactor(BlendFactorMap.at(blend_mode.alphaSrc))
		.setDstAlphaBlendFactor(BlendFactorMap.at(blend_mode.alphaDst))
		.setAlphaBlendOp(BlendFuncMap.at(blend_mode.alphaFunc));

	std::vector<uint32_t> blend_enable_array;
	std::vector<vk::ColorComponentFlags> color_mask_array;
	std::vector<vk::ColorBlendEquationEXT> color_blend_equation_array;

	if (gContext.render_targets.empty())
	{
		blend_enable_array = { gContext.blend_mode.has_value() };
		color_mask_array = { color_mask };
		color_blend_equation_array = { color_blend_equation };
	}
	else
	{
		for (size_t i = 0; i < gContext.render_targets.size(); i++)
		{
			blend_enable_array.push_back(gContext.blend_mode.has_value());
			color_mask_array.push_back(color_mask);
			color_blend_equation_array.push_back(color_blend_equation);
		}
	}

	cmdlist.setColorBlendEnableEXT(0, blend_enable_array);
	cmdlist.setColorWriteMaskEXT(0, color_mask_array);
	cmdlist.setColorBlendEquationEXT(0, color_blend_equation_array);
}
//=============================================================================
void EnsureDepthMode(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.depth_mode_dirty)
		return;

	gContext.depth_mode_dirty = false;

	if (gContext.depth_mode.has_value())
	{
		cmdlist.setDepthTestEnable(true);
		cmdlist.setDepthWriteEnable(gContext.depth_mode.value().writeMask);
		cmdlist.setDepthCompareOp(CompareOpMap.at(gContext.depth_mode.value().func));
	}
	else
	{
		cmdlist.setDepthTestEnable(false);
		cmdlist.setDepthWriteEnable(false);
		cmdlist.setDepthCompareOp(vk::CompareOp::eAlways);
	}
}
//=============================================================================
void EnsureStencilMode(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.stencil_mode_dirty)
		return;

	gContext.stencil_mode_dirty = false;

	cmdlist.setStencilTestEnable(gContext.stencil_mode.has_value());
}
//=============================================================================
void EnsureGraphicsPipelineState(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.pipeline_state_dirty)
		return;

	gContext.pipeline_state_dirty = false;

	if (!gContext.pipeline_states.contains(gContext.pipeline_state))
	{
		auto pipeline = CreateGraphicsPipeline(gContext.pipeline_state);
		gContext.pipeline_states.insert({ gContext.pipeline_state, std::move(pipeline) });
	}

	const auto& pipeline = gContext.pipeline_states.at(gContext.pipeline_state);
	cmdlist.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

	gContext.graphics_pipeline_ignore_bindings.clear();
}
//=============================================================================
void EnsureRaytracingPipelineState(vk::raii::CommandBuffer& cmdlist)
{
	if (!gContext.raytracing_pipeline_states.contains(gContext.raytracing_pipeline_state))
	{
		auto pipeline = CreateRaytracingPipeline(gContext.raytracing_pipeline_state);
		gContext.raytracing_pipeline_states.insert({ gContext.raytracing_pipeline_state, std::move(pipeline) });
	}

	const auto& pipeline = gContext.raytracing_pipeline_states.at(gContext.raytracing_pipeline_state);
	cmdlist.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *pipeline);
}
//=============================================================================
void EnsureGraphicsDescriptors(vk::raii::CommandBuffer& cmdlist)
{
	const auto& pipeline_layout = gContext.pipeline_state.shader->GetPipelineLayout();
	const auto& required_descriptor_bindings = gContext.pipeline_state.shader->GetRequiredDescriptorBindings();
	auto& ignore_bindings = gContext.graphics_pipeline_ignore_bindings;

	PushDescriptors(cmdlist, vk::PipelineBindPoint::eGraphics, pipeline_layout, required_descriptor_bindings, ignore_bindings);

	for (const auto& descriptor_binding : required_descriptor_bindings)
	{
		ignore_bindings.insert(descriptor_binding.binding);
	}
}
//=============================================================================
void EnsureRaytracingDescriptors(vk::raii::CommandBuffer& cmdlist)
{
	const auto& pipeline_layout = gContext.raytracing_pipeline_state.shader->GetPipelineLayout();
	const auto& required_descriptor_bindings = gContext.raytracing_pipeline_state.shader->GetRequiredDescriptorBindings();

	PushDescriptors(cmdlist, vk::PipelineBindPoint::eRayTracingKHR, pipeline_layout, required_descriptor_bindings);
}
//=============================================================================
void EnsureGraphicsState(bool draw_indexed)
{
	auto& cmdlist = gContext.getCurrentFrame().command_buffer;

	EnsureMemoryState(cmdlist, vk::PipelineStageFlagBits2::eAllGraphics);
	EnsureGraphicsPipelineState(cmdlist);
	EnsureGraphicsDescriptors(cmdlist);
	EnsureVertexBuffers(cmdlist);

	if (draw_indexed)
		EnsureIndexBuffer(cmdlist);

	EnsureTopology(cmdlist);
	EnsureViewport(cmdlist);
	EnsureScissor(cmdlist);
	EnsureCullMode(cmdlist);
	EnsureFrontFace(cmdlist);
	EnsureBlendMode(cmdlist);
	EnsureDepthMode(cmdlist);
	EnsureStencilMode(cmdlist);
	EnsureRenderPassActivated();
}
//=============================================================================
void EnsureRaytracingState()
{
	auto& cmdlist = gContext.getCurrentFrame().command_buffer;

	EnsureRenderPassDeactivated();
	EnsureMemoryState(cmdlist, vk::PipelineStageFlagBits2::eAllGraphics);
	EnsureRaytracingPipelineState(cmdlist);
	EnsureRaytracingDescriptors(cmdlist);
}
//=============================================================================
void WaitForGpu()
{
	const auto& fence = gContext.getCurrentFrame().fence;
	auto wait_result = gContext.device.waitForFences({ *fence }, true, UINT64_MAX);
	ReleaseStaging();
}
//=============================================================================
void CreateSwapchain(uint32_t width, uint32_t height)
{
	auto surface_capabilities = gContext.physicalDevice.getSurfaceCapabilitiesKHR(*gContext.surface);

	// https://github.com/nvpro-samples/nvpro_core/blob/f2c05e161bba9ab9a8c96c0173bf0edf7c168dfa/nvvk/swapchain_vk.cpp#L143
	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):

	uint32_t desired_number_of_swapchain_images = surface_capabilities.minImageCount + 1;

	if ((surface_capabilities.maxImageCount > 0) && (desired_number_of_swapchain_images > surface_capabilities.maxImageCount))
	{
		// Application must settle for fewer images than desired:
		desired_number_of_swapchain_images = surface_capabilities.maxImageCount;
	}

	auto max_width = surface_capabilities.maxImageExtent.width;
	auto max_height = surface_capabilities.maxImageExtent.height;

	gContext.width = glm::min(width, max_width);
	gContext.height = glm::min(height, max_height);

	auto image_extent = vk::Extent2D()
		.setWidth(gContext.width)
		.setHeight(gContext.height);

	auto format = gContext.surfaceFormat.format;

	auto swapchain_info = vk::SwapchainCreateInfoKHR()
		.setSurface(*gContext.surface)
		.setMinImageCount(desired_number_of_swapchain_images)
		.setImageFormat(format)
		.setImageColorSpace(gContext.surfaceFormat.colorSpace)
		.setImageExtent(image_extent)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setImageArrayLayers(1)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndices(gContext.queueFamilyIndex)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(true)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setOldSwapchain(*gContext.swapChain);

	gContext.swapChain = gContext.device.createSwapchainKHR(swapchain_info);

	auto backbuffers = gContext.swapChain.getImages();

	gContext.frames.clear();

	for (auto& backbuffer : backbuffers)
	{
		auto frame = RenderContext::Frame();

		auto fence_info = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);

		frame.fence = gContext.device.createFence(fence_info);

		frame.image_acquired_semaphore = gContext.device.createSemaphore({});
		frame.render_complete_semaphore = gContext.device.createSemaphore({});

		auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(*gContext.commandPool);

		auto command_buffers = gContext.device.allocateCommandBuffers(command_buffer_allocate_info);

		frame.command_buffer = std::move(command_buffers.at(0));

		frame.swapchain_texture = std::make_shared<TextureVK>(gContext.width, gContext.height, format, backbuffer);
		frame.swapchain_target = std::make_shared<RenderTargetVK>(gContext.width, gContext.height, frame.swapchain_texture.get());

		gContext.frames.push_back(std::move(frame));
	}

	gContext.frame_index = 0;
	gContext.semaphore_index = 0;
}
//=============================================================================
void MoveToNextFrame()
{
	const auto& image_acquired_semaphore = gContext.frames.at(gContext.semaphore_index).image_acquired_semaphore;

	auto [result, image_index] = gContext.swapChain.acquireNextImage(UINT64_MAX, *image_acquired_semaphore);

	gContext.frame_index = image_index;
}
//=============================================================================
void RenderBegin()
{
	assert(!gContext.working);
	gContext.working = true;

	gContext.pipeline_state_dirty = true;
	gContext.topology_dirty = true;
	gContext.viewport_dirty = true;
	gContext.scissor_dirty = true;
	gContext.cull_mode_dirty = true;
	gContext.front_face_dirty = true;
	gContext.vertex_buffers_dirty = true;
	gContext.index_buffer_dirty = true;
	gContext.blend_mode_dirty = true;
	gContext.depth_mode_dirty = true;
	gContext.stencil_mode_dirty = true;

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	gContext.getCurrentFrame().command_buffer.begin(begin_info);
}
//=============================================================================
void RenderEnd()
{
	assert(gContext.working);
	gContext.working = false;

	EnsureRenderPassDeactivated();

	gContext.getCurrentFrame().swapchain_texture->EnsureState(gContext.getCurrentFrame().command_buffer,
		vk::ImageLayout::ePresentSrcKHR);

	gContext.getCurrentFrame().command_buffer.end();

	const auto& frame = gContext.getCurrentFrame();

	gContext.device.resetFences({ *frame.fence });

	auto wait_dst_stage_mask = vk::PipelineStageFlags{
		vk::PipelineStageFlagBits::eAllCommands
	};

	auto submit_info = vk::SubmitInfo()
		.setWaitDstStageMask(wait_dst_stage_mask)
		.setWaitSemaphores(*frame.image_acquired_semaphore)
		.setCommandBuffers(*frame.command_buffer)
		.setSignalSemaphores(*frame.render_complete_semaphore);

	gContext.queue.submit(submit_info, *frame.fence);
}
//=============================================================================
#endif // RENDER_VULKAN