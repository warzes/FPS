#include "stdafx.h"
#if RENDER_VULKAN
#include "RenderSystem.h"
#include "ContextVK.h"
#include "RenderTargetVK.h"
#include "ShaderVK.h"
#include "BufferVK.h"
#include "AccelerationStructureVK.h"
#include "Log.h"
//=============================================================================
#if defined(_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* /*pUserData*/)
{
#if !defined( NDEBUG )
	if (pCallbackData->messageIdNumber == 648835635)
	{
		// UNASSIGNED-khronos-Validation-debug-build-warning-message
		return VK_FALSE;
	}
	if (pCallbackData->messageIdNumber == 767975156)
	{
		// UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension
		return VK_FALSE;
	}
#endif

	std::string msg = vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": "
		+ vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n";
	msg += std::string("\t") + "messageIdName   = <" + std::string(pCallbackData->pMessageIdName) + ">\n";
	msg += std::string("\t") + "messageIdNumber = " + std::to_string(pCallbackData->messageIdNumber) + "\n";
	msg += std::string("\t") + "message         = <" + std::string(pCallbackData->pMessage) + ">\n";
	if (pCallbackData->queueLabelCount > 0)
	{
		msg += std::string("\t") + "Queue Labels:\n";
		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
		{
			msg += std::string("\t\t") + "labelName = <" + std::string(pCallbackData->pQueueLabels[i].pLabelName) + ">\n";
		}
	}
	if (pCallbackData->cmdBufLabelCount > 0)
	{
		msg += std::string("\t") + "CommandBuffer Labels:\n";
		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
		{
			msg += std::string("\t\t") + "labelName = <" + std::string(pCallbackData->pCmdBufLabels[i].pLabelName) + ">\n";
		}
	}
	if (pCallbackData->objectCount > 0)
	{
		msg += std::string("\t") + "Objects:\n";
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
		{
			msg += std::string("\t\t") + "Object " + std::to_string(i) + "\n";
			msg += std::string("\t\t\t") + "objectType   = " + vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n";
			msg += std::string("\t\t\t") + "objectHandle = " + std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";
			if (pCallbackData->pObjects[i].pObjectName)
			{
				msg += std::string("\t\t\t") + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
			}
		}
	}
	return VK_TRUE;
}
#endif
//=============================================================================
RenderContext gContext{};
//=============================================================================
void RenderSystem::SetTopology(Topology topology)
{
	gContext.topology = topology;
	gContext.topology_dirty = true;
}
//=============================================================================
void RenderSystem::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::SetScissor(std::optional<Scissor> scissor)
{
	gContext.scissor = scissor;
	gContext.scissor_dirty = true;
}
//=============================================================================





bool RenderSystem::createAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	if (!gContext.Create()) return false;

	auto all_extensions = gContext.context.enumerateInstanceExtensionProperties();

	for (auto extension : all_extensions)
	{
		//	std::cout << extension.extensionName << std::endl;
	}

	auto all_layers = gContext.context.enumerateInstanceLayerProperties();

	for (auto layer : all_layers)
	{
		//	std::cout << layer.layerName << std::endl;
	}

	auto extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if PLATFORM_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(_DEBUG) && 0
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

#if defined(_DEBUG) && 0
	auto layers = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif

	auto version = gContext.context.enumerateInstanceVersion();

	auto major_version = VK_API_VERSION_MAJOR(version);
	auto minor_version = VK_API_VERSION_MINOR(version);
	auto patch_version = VK_API_VERSION_PATCH(version);

	//std::cout << "available vulkan version: " << major_version << "." << minor_version << std::endl;

	auto application_info = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_3);

	auto instance_create_info = vk::InstanceCreateInfo()
		.setPEnabledExtensionNames(extensions)
#if defined(_DEBUG) && 0
		.setPEnabledLayerNames(layers)
#endif
		.setPApplicationInfo(&application_info);

#if defined(_DEBUG) && 0
	auto debug_utils_messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
		)
		.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		)
		.setPfnUserCallback(&DebugUtilsMessengerCallback);

	auto enabled_validation_features = {
		//	vk::ValidationFeatureEnableEXT::eBestPractices,
			vk::ValidationFeatureEnableEXT::eDebugPrintf,
			//	vk::ValidationFeatureEnableEXT::eGpuAssisted,
			//	vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot,
			//	vk::ValidationFeatureEnableEXT::eSynchronizationValidation
	};

	auto validation_features = vk::ValidationFeaturesEXT()
		.setEnabledValidationFeatures(enabled_validation_features);
#endif

	auto instance_create_info_chain = vk::StructureChain<
		vk::InstanceCreateInfo
#if defined(_DEBUG) && 0
		,
		vk::DebugUtilsMessengerCreateInfoEXT,
		vk::ValidationFeaturesEXT
#endif
	>(
		instance_create_info
#if defined(_DEBUG) && 0
		,
		debug_utils_messenger_create_info,
		validation_features
#endif
	);

	gContext.instance = gContext.context.createInstance(instance_create_info_chain.get<vk::InstanceCreateInfo>());

#if defined(_DEBUG) && 0
	gContext.debug_utils_messenger = gContext.instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
#endif

	auto devices = gContext.instance.enumeratePhysicalDevices();
	size_t device_index = 0;
	auto preferred_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
	for (size_t i = 0; i < devices.size(); i++)
	{
		auto properties = devices.at(i).getProperties();
		if (properties.deviceType == preferred_device_type)
		{
			device_index = i;
			break;
		}
	}

	gContext.physical_device = std::move(devices.at(device_index));

	auto properties = gContext.physical_device.getQueueFamilyProperties();

	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			gContext.queue_family_index = static_cast<uint32_t>(i);
			break;
		}
	}

	auto all_device_extensions = gContext.physical_device.enumerateDeviceExtensionProperties();

	for (auto device_extension : all_device_extensions)
	{
		//	std::cout << device_extension.extensionName << std::endl;
	}

	std::vector device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,

		// dynamic pipeline
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
	};

	if (createInfo.features.contains(RenderFeature::Raytracing))
	{
		device_extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		device_extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		device_extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	}

	auto queue_priority = { 1.0f };

	auto queue_info = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(gContext.queue_family_index)
		.setQueuePriorities(queue_priority);

	auto default_device_features = gContext.physical_device.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT
	>();

	auto raytracing_device_features = gContext.physical_device.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT,
		vk::PhysicalDeviceBufferAddressFeaturesEXT,
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR
	>();

	auto device_info = vk::DeviceCreateInfo()
		.setQueueCreateInfos(queue_info)
		.setPEnabledExtensionNames(device_extensions)
		.setPEnabledFeatures(nullptr);

	if (createInfo.features.contains(RenderFeature::Raytracing))
		device_info.setPNext(&raytracing_device_features.get<vk::PhysicalDeviceFeatures2>());
	else
		device_info.setPNext(&default_device_features.get<vk::PhysicalDeviceFeatures2>());

	gContext.device = gContext.physical_device.createDevice(device_info);

	gContext.queue = gContext.device.getQueue(gContext.queue_family_index, 0);

#if PLATFORM_WINDOWS
	auto surface_info = vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(data.hwnd);
#endif

	gContext.surface = vk::raii::SurfaceKHR(gContext.instance, surface_info);

	auto formats = gContext.physical_device.getSurfaceFormatsKHR(*gContext.surface);

	if ((formats.size() == 1) && (formats.at(0).format == vk::Format::eUndefined))
	{
		gContext.surface_format = {
			vk::Format::eB8G8R8A8Unorm,
			formats.at(0).colorSpace
		};
	}
	else
	{
		bool found = false;
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm)
			{
				gContext.surface_format = format;
				found = true;
				break;
			}
		}
		if (!found)
		{
			gContext.surface_format = formats.at(0);
		}
	}

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(gContext.queue_family_index);

	gContext.command_pool = gContext.device.createCommandPool(command_pool_info);

	gContext.pipeline_state.color_attachment_formats = { gContext.surface_format.format };
	gContext.pipeline_state.depth_stencil_format = DefaultDepthStencilFormat;

	CreateSwapchain(data.width, data.height);
	MoveToNextFrame();
	RenderBegin();

	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
	RenderEnd();
	WaitForGpu();
	gContext.Destroy();
}
//=============================================================================
void RenderSystem::resize(uint32_t width, uint32_t height)
{
	RenderEnd();
	WaitForGpu();
	CreateSwapchain(width, height);
	MoveToNextFrame();
	RenderBegin();
}
//=============================================================================
void RenderSystem::present()
{
	RenderEnd();

	const auto& render_complete_semaphore = gContext.getCurrentFrame().render_complete_semaphore;

	auto present_info = vk::PresentInfoKHR()
		.setWaitSemaphores(*render_complete_semaphore)
		.setSwapchains(*gContext.swapchain)
		.setImageIndices(gContext.frame_index);

	auto present_result = gContext.queue.presentKHR(present_info);

	gContext.semaphore_index = (gContext.semaphore_index + 1) % gContext.frames.size();

	MoveToNextFrame();
	WaitForGpu();
	RenderBegin();
}
//=============================================================================

//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, TextureHandle* handle)
{
	gContext.textures[binding] = (TextureVK*)handle;
	gContext.graphics_pipeline_ignore_bindings.erase(binding);
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, const TextureHandle* handle)
{
	gContext.textures[binding] = (TextureVK*)handle;
	gContext.graphics_pipeline_ignore_bindings.erase(binding);
}
//=============================================================================
void RenderSystem::SetRenderTarget(std::nullopt_t)
{
	SetRenderTarget({}); // TODO:
}
//=============================================================================
void RenderSystem::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	std::vector<RenderTargetVK*> render_targets;
	std::vector<vk::Format> color_attachment_formats;
	std::optional<vk::Format> depth_stencil_format;

	if (count == 0)
	{
		color_attachment_formats = { gContext.surface_format.format };
		depth_stencil_format = DefaultDepthStencilFormat;
	}
	else
	{
		for (size_t i = 0; i < count; i++)
		{
			auto target = (RenderTargetVK*)(RenderTargetHandle*)*(RenderTarget*)render_target[i];
			render_targets.push_back(target);
			color_attachment_formats.push_back(target->GetTexture()->GetFormat());

			if (!depth_stencil_format.has_value())
				depth_stencil_format = target->GetDepthStencilFormat();
		}
	}

	if (gContext.render_targets.size() != render_targets.size())
		gContext.blend_mode_dirty = true;

	gContext.pipeline_state_dirty = true;
	gContext.pipeline_state.color_attachment_formats = color_attachment_formats;
	gContext.pipeline_state.depth_stencil_format = depth_stencil_format;
	gContext.render_targets = render_targets;
	EnsureRenderPassDeactivated();

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;

	if (!gContext.scissor.has_value())
		gContext.scissor_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(ShaderHandle* handle)
{
	gContext.pipeline_state.shader = (ShaderVK*)handle;
	gContext.pipeline_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(const ShaderHandle* handle)
{
	gContext.pipeline_state.shader = (ShaderVK*)handle;
	gContext.pipeline_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.pipeline_state.input_layouts = value;
	gContext.pipeline_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	gContext.vertex_buffers.clear();
	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferVK*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		gContext.vertex_buffers.push_back(buffer);
	}
	gContext.vertex_buffers_dirty = true;
}
//=============================================================================
void RenderSystem::SetIndexBuffer(IndexBufferHandle* handle)
{
	gContext.index_buffer = (IndexBufferVK*)handle;
	gContext.index_buffer_dirty = true;
}
//=============================================================================
void RenderSystem::SetIndexBuffer(const IndexBufferHandle* handle)
{
	gContext.index_buffer = (IndexBufferVK*)handle;
	gContext.index_buffer_dirty = true;
}
//=============================================================================
void RenderSystem::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	gContext.uniform_buffers[binding] = (UniformBufferVK*)handle;
	gContext.graphics_pipeline_ignore_bindings.erase(binding);
}
//=============================================================================
void RenderSystem::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	gContext.blend_mode = blend_mode;
	gContext.blend_mode_dirty = true;
}
//=============================================================================
void RenderSystem::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	gContext.depth_mode = depth_mode;
	gContext.depth_mode_dirty = true;
}
//=============================================================================
void RenderSystem::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	gContext.stencil_mode_dirty = true;
	gContext.stencil_mode = stencil_mode;
}
//=============================================================================
void RenderSystem::SetCullMode(CullMode cull_mode)
{
	gContext.cull_mode = cull_mode;
	gContext.cull_mode_dirty = true;
}
//=============================================================================
void RenderSystem::SetSampler(Sampler value)
{
	gContext.sampler_state.sampler = value;
}
//=============================================================================
void RenderSystem::SetTextureAddress(TextureAddress value)
{
	gContext.sampler_state.texture_address = value;
}
//=============================================================================
void RenderSystem::SetFrontFace(FrontFace value)
{
	gContext.front_face = value;
	gContext.front_face_dirty = true;
}
//=============================================================================
void RenderSystem::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
}
//=============================================================================
void RenderSystem::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	EnsureRenderPassActivated();

	auto width = gContext.GetBackbufferWidth();
	auto height = gContext.GetBackbufferHeight();

	auto clear_rect = vk::ClearRect()
		.setBaseArrayLayer(0)
		.setLayerCount(1)
		.setRect({ { 0, 0 }, { width, height } });

	if (color.has_value())
	{
		auto value = color.value();

		auto clear_color_value = vk::ClearColorValue()
			.setFloat32({ value.r, value.g, value.b, value.a });

		auto clear_value = vk::ClearValue()
			.setColor(clear_color_value);

		auto attachment = vk::ClearAttachment()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setColorAttachment(0) // TODO: clear all attachments
			.setClearValue(clear_value);

		gContext.getCurrentFrame().command_buffer.clearAttachments({ attachment }, { clear_rect });
	}

	if (depth.has_value() || stencil.has_value())
	{
		auto clear_depth_stencil_value = vk::ClearDepthStencilValue()
			.setDepth(depth.value_or(1.0f))
			.setStencil((uint32_t)stencil.value_or(0)); // TODO: maybe we should change argument uint8_t -> uint32_t

		auto clear_value = vk::ClearValue()
			.setDepthStencil(clear_depth_stencil_value);

		auto aspect_mask = vk::ImageAspectFlags();

		if (depth.has_value())
			aspect_mask |= vk::ImageAspectFlagBits::eDepth;

		if (stencil.has_value())
			aspect_mask |= vk::ImageAspectFlagBits::eStencil;

		auto attachment = vk::ClearAttachment()
			.setAspectMask(aspect_mask)
			.setColorAttachment(0)
			.setClearValue(clear_value);

		gContext.getCurrentFrame().command_buffer.clearAttachments({ attachment }, { clear_rect });
	}
}
//=============================================================================
void RenderSystem::Draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count)
{
	EnsureGraphicsState(false);
	gContext.getCurrentFrame().command_buffer.draw(vertex_count, instance_count, vertex_offset, 0);
}
//=============================================================================
void RenderSystem::DrawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count)
{
	EnsureGraphicsState(true);
	gContext.getCurrentFrame().command_buffer.drawIndexed(index_count, instance_count, index_offset, 0, 0);
}
//=============================================================================
void RenderSystem::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dst_texture_handle)
{
	auto dst_texture = (TextureVK*)dst_texture_handle;
	auto dst_format = gContext.GetBackbufferFormat();

	assert(dst_texture->GetWidth() == size.x);
	assert(dst_texture->GetHeight() == size.y);
	assert(dst_texture->GetFormat() == dst_format);

	if (size.x <= 0 || size.y <= 0)
		return;

	EnsureRenderPassDeactivated();

	auto pos_x = static_cast<int32_t>(pos.x);
	auto pos_y = static_cast<int32_t>(pos.y);
	auto width = static_cast<uint32_t>(size.x);
	auto height = static_cast<uint32_t>(size.y);

	auto src_target = !gContext.render_targets.empty() ? gContext.render_targets.at(0) : gContext.getCurrentFrame().swapchain_target.get();
	auto src_texture = src_target->GetTexture();
	auto src_image = src_texture->GetImage();
	auto dst_image = dst_texture->GetImage();

	auto subresource = vk::ImageSubresourceLayers()
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setLayerCount(1);

	auto region = vk::ImageCopy2()
		.setSrcSubresource(subresource)
		.setDstSubresource(subresource)
		.setSrcOffset({ pos_x, pos_y, 0 })
		.setDstOffset({ 0, 0, 0 })
		.setExtent({ width, height, 1 });

	src_texture->EnsureState(gContext.getCurrentFrame().command_buffer, vk::ImageLayout::eTransferSrcOptimal);
	dst_texture->EnsureState(gContext.getCurrentFrame().command_buffer, vk::ImageLayout::eTransferDstOptimal);

	auto copy_image_info = vk::CopyImageInfo2()
		.setSrcImage(src_image)
		.setDstImage(dst_image)
		.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
		.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
		.setRegions(region);

	gContext.getCurrentFrame().command_buffer.copyImage2(copy_image_info);
}
//=============================================================================
void RenderSystem::SetRaytracingShader(RaytracingShaderHandle* handle)
{
	auto shader = (RaytracingShaderVK*)handle;
	gContext.raytracing_pipeline_state.shader = shader;
}
//=============================================================================
void RenderSystem::SetStorageBuffer(uint32_t binding, StorageBufferHandle* handle)
{
	gContext.storage_buffers[binding] = (StorageBufferVK*)handle;
}
//=============================================================================
void RenderSystem::SetAccelerationStructure(uint32_t binding, TopLevelAccelerationStructureHandle* handle)
{
	gContext.top_level_acceleration_structures[binding] = (TopLevelAccelerationStructureVK*)handle;
}
//=============================================================================
void RenderSystem::DispatchRays(uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!gContext.render_targets.empty());

	EnsureRaytracingState();

	const auto& pipeline = gContext.raytracing_pipeline_states.at(gContext.raytracing_pipeline_state);
	static auto binding_table = CreateRaytracingShaderBindingTable(pipeline);

	gContext.getCurrentFrame().command_buffer.traceRaysKHR(binding_table.raygen_address, binding_table.miss_address,
		binding_table.hit_address, binding_table.callable_address, width, height, depth);
}
//=============================================================================
#endif // RENDER_VULKAN