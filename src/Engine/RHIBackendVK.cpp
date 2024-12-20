#include "stdafx.h"
#if RENDER_VULKAN
#include "RHIBackend.h"
#include "ContextVK.h"
#include "ShaderVK.h"
#include "TextureVK.h"
#include "RenderTargetVK.h"
#include "BufferVK.h"
#include "AccelerationStructureVK.h"
#include "Log.h"
//=============================================================================
#if RHI_VALIDATION_ENABLED
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
bool RHIBackend::CreateAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	auto extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if PLATFORM_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if RHI_VALIDATION_ENABLED
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};

#if RHI_VALIDATION_ENABLED
	auto layers = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif

	auto version = gContext.context.enumerateInstanceVersion();

	auto major_version = VK_API_VERSION_MAJOR(version);
	auto minor_version = VK_API_VERSION_MINOR(version);
	auto patch_version = VK_API_VERSION_PATCH(version);

	Print("Available vulkan version: "
		+ std::to_string(major_version) + "."
		+ std::to_string(minor_version) + "."
		+ std::to_string(patch_version));

	auto application_info = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_3);

	auto instance_create_info = vk::InstanceCreateInfo()
		.setPEnabledExtensionNames(extensions)
#if RHI_VALIDATION_ENABLED
		.setPEnabledLayerNames(layers)
#endif
		.setPApplicationInfo(&application_info);

#if RHI_VALIDATION_ENABLED
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
#if RHI_VALIDATION_ENABLED
		,
		vk::DebugUtilsMessengerCreateInfoEXT,
		vk::ValidationFeaturesEXT
#endif
	>(
		instance_create_info
#if RHI_VALIDATION_ENABLED
		,
		debug_utils_messenger_create_info,
		validation_features
#endif
	);

	gContext.instance = gContext.context.createInstance(instance_create_info_chain.get<vk::InstanceCreateInfo>());

#if RHI_VALIDATION_ENABLED
	gContext.debugUtilsMessenger = gContext.instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
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

	gContext.physicalDevice = std::move(devices.at(device_index));

	auto properties = gContext.physicalDevice.getQueueFamilyProperties();

	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			gContext.queueFamilyIndex = static_cast<uint32_t>(i);
			break;
		}
	}

	auto all_device_extensions = gContext.physicalDevice.enumerateDeviceExtensionProperties();

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
		.setQueueFamilyIndex(gContext.queueFamilyIndex)
		.setQueuePriorities(queue_priority);

	auto default_device_features = gContext.physicalDevice.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT
	>();

	auto raytracing_device_features = gContext.physicalDevice.getFeatures2<
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

	gContext.device = gContext.physicalDevice.createDevice(device_info);

	gContext.queue = gContext.device.getQueue(gContext.queueFamilyIndex, 0);

#if PLATFORM_WINDOWS
	auto surface_info = vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(data.hwnd);
#endif

	gContext.surface = vk::raii::SurfaceKHR(gContext.instance, surface_info);

	auto formats = gContext.physicalDevice.getSurfaceFormatsKHR(*gContext.surface);

	if ((formats.size() == 1) && (formats.at(0).format == vk::Format::eUndefined))
	{
		gContext.surfaceFormat = {
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
				gContext.surfaceFormat = format;
				found = true;
				break;
			}
		}
		if (!found)
		{
			gContext.surfaceFormat = formats.at(0);
		}
	}

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(gContext.queueFamilyIndex);

	gContext.commandPool = gContext.device.createCommandPool(command_pool_info);

	gContext.pipeline_state.color_attachment_formats = { gContext.surfaceFormat.format };
	gContext.pipeline_state.depth_stencil_format = DefaultDepthStencilFormat;

	CreateSwapchain(data.width, data.height);
	MoveToNextFrame();
	RenderBegin();

	return true;
}
//=============================================================================
void RHIBackend::DestroyAPI()
{
	RenderEnd();
	WaitForGpu();
	gContext.Clear();
}
//=============================================================================
void RHIBackend::ResizeFrameBuffer(uint32_t width, uint32_t height)
{
	RenderEnd();
	WaitForGpu();
	CreateSwapchain(width, height);
	MoveToNextFrame();
	RenderBegin();
}
//=============================================================================
void RHIBackend::Present()
{
	RenderEnd();

	const auto& render_complete_semaphore = gContext.getCurrentFrame().render_complete_semaphore;

	auto present_info = vk::PresentInfoKHR()
		.setWaitSemaphores(*render_complete_semaphore)
		.setSwapchains(*gContext.swapChain)
		.setImageIndices(gContext.frame_index);

	auto present_result = gContext.queue.presentKHR(present_info);

	gContext.semaphore_index = (gContext.semaphore_index + 1) % gContext.frames.size();

	MoveToNextFrame();
	WaitForGpu();
	RenderBegin();
}
//=============================================================================
void RHIBackend::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	EnsureRenderPassActivated();

	auto width = gContext.GetBackBufferWidth();
	auto height = gContext.GetBackBufferHeight();

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
void RHIBackend::Draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState(false);
	gContext.getCurrentFrame().command_buffer.draw(vertexCount, instanceCount, vertexOffset, 0);
}
//=============================================================================
void RHIBackend::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState(true);
	gContext.getCurrentFrame().command_buffer.drawIndexed(indexCount, instanceCount, indexOffset, 0, 0);
}
//=============================================================================
void RHIBackend::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dstTextureHandle)
{
	auto dst_texture = (TextureVK*)dstTextureHandle;
	auto dst_format = gContext.GetBackBufferFormat();

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
ShaderHandle* RHIBackend::CreateShader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines)
{
	auto shader = new ShaderVK(vertexCode, fragmentCode, defines);
	gContext.objects.insert(shader);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RHIBackend::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderVK*)handle;

	for (auto& [state, pipeline] : gContext.pipeline_states)
	{
		if (state.shader != shader)
			continue;

		DestroyStaging(std::move(pipeline));
	}

	std::erase_if(gContext.pipeline_states, [&](const auto& item) {
		const auto& [state, pipeline] = item;
		return state.shader == shader;
		});

	gContext.objects.erase(shader);
	delete shader;
}
//=============================================================================
TextureHandle* RHIBackend::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureVK(width, height, PixelFormatMap.at(format), mip_count);
	gContext.objects.insert(texture);
	return (TextureHandle*)texture;
}
//=============================================================================
void RHIBackend::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureVK*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RHIBackend::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureVK*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RHIBackend::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureVK*)handle;

	std::erase_if(gContext.textures, [&](const auto& item) {
		const auto& [binding, _texture] = item;
		return texture == _texture;
		});

	gContext.objects.erase(texture);
	delete texture;
}
//=============================================================================
RenderTargetHandle* RHIBackend::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureVK*)texture_handle;
	auto render_target = new RenderTargetVK(width, height, texture);
	gContext.objects.insert(render_target);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RHIBackend::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetVK*)handle;
	gContext.objects.erase(render_target);
	delete render_target;
}
//=============================================================================
VertexBufferHandle* RHIBackend::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferVK(size, stride);
	gContext.objects.insert(buffer);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferVK*)handle;
	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferVK*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	auto has_buffer = std::ranges::any_of(gContext.vertex_buffers, [&](auto vertex_buffer) {
		return vertex_buffer == buffer;
		});

	if (has_buffer)
		gContext.vertexBuffersDirty = true;
}
//=============================================================================
IndexBufferHandle* RHIBackend::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferVK(size, stride);
	gContext.objects.insert(buffer);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferVK*)handle;
	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferVK*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	if (gContext.index_buffer == buffer)
		gContext.indexBufferDirty = true;
}
//=============================================================================
UniformBufferHandle* RHIBackend::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferVK(size);
	gContext.objects.insert(buffer);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferVK*)handle;

	std::erase_if(gContext.uniform_buffers, [&](const auto& item) {
		const auto& [binding, _buffer] = item;
		return buffer == _buffer;
		});

	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferVK*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RHIBackend::SetRasterizerState(const RasterizerState& state)
{

}
//=============================================================================
void RHIBackend::SetSamplerState(const SamplerState& state)
{

}
//=============================================================================
void RHIBackend::SetTopology(Topology topology)
{
	gContext.topology = topology;
	gContext.topologyDirty = true;
}
//=============================================================================
void RHIBackend::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewportDirty = true;
}
//=============================================================================
void RHIBackend::SetScissor(std::optional<Scissor> scissor)
{
	gContext.scissor = scissor;
	gContext.scissorDirty = true;
}
//=============================================================================
void RHIBackend::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	gContext.blendMode = blend_mode;
	gContext.blendModeDirty = true;
}
//=============================================================================
void RHIBackend::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	gContext.depth_mode = depth_mode;
	gContext.depthModeDirty = true;
}
//=============================================================================
void RHIBackend::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	gContext.stencilModeDirty = true;
	gContext.stencil_mode = stencil_mode;
}
//=============================================================================
void RHIBackend::SetCullMode(CullMode cull_mode)
{
	gContext.cull_mode = cull_mode;
	gContext.cullModeDirty = true;
}
//=============================================================================
void RHIBackend::SetSamplerFilter(Filter value)
{
	gContext.samplerState.filter = value;
}
//=============================================================================
void RHIBackend::SetTextureAddress(TextureAddress value)
{
	gContext.samplerState.texture_address = value;
}
//=============================================================================
void RHIBackend::SetFrontFace(FrontFace value)
{
	gContext.front_face = value;
	gContext.frontFaceDirty = true;
}
//=============================================================================
void RHIBackend::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
}
//=============================================================================
void RHIBackend::SetShader(ShaderHandle* handle)
{
	gContext.pipeline_state.shader = (ShaderVK*)handle;
	gContext.pipelineStateDirty = true;
}
//=============================================================================
void RHIBackend::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.pipeline_state.inputLayouts = value;
	gContext.pipelineStateDirty = true;
}
//=============================================================================
void RHIBackend::SetTexture(uint32_t binding, TextureHandle* handle)
{
	gContext.textures[binding] = (TextureVK*)handle;
	gContext.graphics_pipeline_ignore_bindings.erase(binding);
}
//=============================================================================
void RHIBackend::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	std::vector<RenderTargetVK*> render_targets;
	std::vector<vk::Format> color_attachment_formats;
	std::optional<vk::Format> depth_stencil_format;

	if (count == 0)
	{
		color_attachment_formats = { gContext.surfaceFormat.format };
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
		gContext.blendModeDirty = true;

	gContext.pipelineStateDirty = true;
	gContext.pipeline_state.color_attachment_formats = color_attachment_formats;
	gContext.pipeline_state.depth_stencil_format = depth_stencil_format;
	gContext.render_targets = render_targets;
	EnsureRenderPassDeactivated();

	if (!gContext.viewport.has_value())
		gContext.viewportDirty = true;

	if (!gContext.scissor.has_value())
		gContext.scissorDirty = true;
}
//=============================================================================
void RHIBackend::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	gContext.vertex_buffers.clear();
	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferVK*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		gContext.vertex_buffers.push_back(buffer);
	}
	gContext.vertexBuffersDirty = true;
}
//=============================================================================
void RHIBackend::SetIndexBuffer(IndexBufferHandle* handle)
{
	gContext.index_buffer = (IndexBufferVK*)handle;
	gContext.indexBufferDirty = true;
}
//=============================================================================
void RHIBackend::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	gContext.uniform_buffers[binding] = (UniformBufferVK*)handle;
	gContext.graphics_pipeline_ignore_bindings.erase(binding);
}
//=============================================================================
RaytracingShaderHandle* RHIBackend::CreateRaytracingShader(const std::string& raygen_code, const std::vector<std::string>& miss_code, const std::string& closesthit_code, const std::vector<std::string>& defines)
{
	auto shader = new RaytracingShaderVK(raygen_code, miss_code, closesthit_code, defines);
	gContext.objects.insert(shader);
	return (RaytracingShaderHandle*)shader;
}
//=============================================================================
void RHIBackend::DestroyRaytracingShader(RaytracingShaderHandle* handle)
{
	auto shader = (RaytracingShaderVK*)handle;

	std::erase_if(gContext.raytracing_pipeline_states, [&](const auto& item) {
		const auto& [state, pipeline] = item;
		return state.shader == shader;
		});

	gContext.objects.erase(shader);
	delete shader;
}
//=============================================================================
BottomLevelAccelerationStructureHandle* RHIBackend::CreateBottomLevelAccelerationStructure(const void* vertex_memory, uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count, uint32_t index_stride, const glm::mat4& transform)
{
	auto bottom_level_acceleration_structure = new BottomLevelAccelerationStructureVK(vertex_memory,
		vertex_count, vertex_stride, index_memory, index_count, index_stride, transform);
	gContext.objects.insert(bottom_level_acceleration_structure);
	return (BottomLevelAccelerationStructureHandle*)bottom_level_acceleration_structure;
}
//=============================================================================
void RHIBackend::DestroyBottomLevelAccelerationStructure(BottomLevelAccelerationStructureHandle* handle)
{
	auto bottom_level_acceleration_structure = (BottomLevelAccelerationStructureVK*)handle;
	gContext.objects.erase(bottom_level_acceleration_structure);
	delete bottom_level_acceleration_structure;
}
//=============================================================================
TopLevelAccelerationStructureHandle* RHIBackend::CreateTopLevelAccelerationStructure(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures)
{
	auto top_level_acceleration_structure = new TopLevelAccelerationStructureVK(bottom_level_acceleration_structures);
	gContext.objects.insert(top_level_acceleration_structure);
	return (TopLevelAccelerationStructureHandle*)top_level_acceleration_structure;
}
//=============================================================================
void RHIBackend::DestroyTopLevelAccelerationStructure(TopLevelAccelerationStructureHandle* handle)
{
	auto top_level_acceleration_structure = (TopLevelAccelerationStructureVK*)handle;

	std::erase_if(gContext.top_level_acceleration_structures, [&](const auto& item) {
		const auto& [binding, _acceleration_structure] = item;
		return top_level_acceleration_structure == _acceleration_structure;
		});

	gContext.objects.erase(top_level_acceleration_structure);
	delete top_level_acceleration_structure;
}
//=============================================================================
StorageBufferHandle* RHIBackend::CreateStorageBuffer(size_t size)
{
	auto buffer = new StorageBufferVK(size);
	gContext.objects.insert(buffer);
	return (StorageBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyStorageBuffer(StorageBufferHandle* handle)
{
	auto buffer = (StorageBufferVK*)handle;

	std::erase_if(gContext.storage_buffers, [&](const auto& item) {
		const auto& [binding, _buffer] = item;
		return buffer == _buffer;
		});

	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteStorageBufferMemory(StorageBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (StorageBufferVK*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RHIBackend::SetStorageBuffer(uint32_t binding, StorageBufferHandle* handle)
{
	gContext.storage_buffers[binding] = (StorageBufferVK*)handle;
}
//=============================================================================
void RHIBackend::SetRaytracingShader(RaytracingShaderHandle* handle)
{
	auto shader = (RaytracingShaderVK*)handle;
	gContext.raytracing_pipeline_state.shader = shader;
}
//=============================================================================
void RHIBackend::SetAccelerationStructure(uint32_t binding, TopLevelAccelerationStructureHandle* handle)
{
	gContext.top_level_acceleration_structures[binding] = (TopLevelAccelerationStructureVK*)handle;
}
//=============================================================================
void RHIBackend::DispatchRays(uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!gContext.render_targets.empty());

	EnsureRaytracingState();

	const auto& pipeline = gContext.raytracing_pipeline_states.at(gContext.raytracing_pipeline_state);
	static auto binding_table = CreateRaytracingShaderBindingTable(pipeline);

	gContext.getCurrentFrame().command_buffer.traceRaysKHR(binding_table.raygen_address, binding_table.miss_address, binding_table.hit_address, binding_table.callable_address, width, height, depth);
}
//=============================================================================
#endif // RENDER_VULKAN