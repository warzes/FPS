#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"
#include "PipelineStateVK.h"

class RenderContext final
{
public:
	void Clear();

	vk::raii::Context                context;
	vk::raii::Instance               instance{ nullptr };
#if defined(_DEBUG)
	vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger{ nullptr };
#endif
	vk::raii::PhysicalDevice         physicalDevice{ nullptr };
	vk::raii::Queue                  queue{ nullptr };
	vk::raii::Device                 device{ nullptr };
	uint32_t                         queueFamilyIndex = -1;
	vk::SurfaceFormatKHR             surfaceFormat;
	vk::raii::SurfaceKHR             surface{ nullptr };
	vk::raii::SwapchainKHR           swapChain{ nullptr };
	vk::raii::CommandPool            commandPool{ nullptr };

	bool working = false;

	uint32_t width = 0;
	uint32_t height = 0;

	struct Frame
	{
		vk::raii::Fence fence = nullptr;
		std::shared_ptr<TextureVK> swapchain_texture;
		std::shared_ptr<RenderTargetVK> swapchain_target;
		vk::raii::Semaphore image_acquired_semaphore = nullptr;
		vk::raii::Semaphore render_complete_semaphore = nullptr;
		vk::raii::CommandBuffer command_buffer = nullptr;
		std::vector<VulkanObject> staging_objects;
	};

	std::vector<Frame> frames;

	uint32_t semaphore_index = 0;
	uint32_t frame_index = 0;

	Frame& getCurrentFrame() { return frames.at(frame_index); }

	std::unordered_map<uint32_t, TextureVK*> textures;
	std::unordered_map<uint32_t, UniformBufferVK*> uniform_buffers;
	std::unordered_map<uint32_t, StorageBufferVK*> storage_buffers;
	std::unordered_map<uint32_t, TopLevelAccelerationStructureVK*> top_level_acceleration_structures;

	std::unordered_map<PipelineStateVK, vk::raii::Pipeline> pipeline_states;

	RaytracingPipelineStateVK raytracing_pipeline_state;
	std::unordered_map<RaytracingPipelineStateVK, vk::raii::Pipeline> raytracing_pipeline_states;

	SamplerStateVK samplerState;
	std::unordered_map<SamplerStateVK, vk::raii::Sampler> samplerStates;

	std::vector<RenderTargetVK*> render_targets;

	PipelineStateVK pipeline_state;
	std::optional<Scissor> scissor;
	std::optional<Viewport> viewport;
	std::optional<DepthMode> depth_mode = DepthMode();
	std::optional<StencilMode> stencil_mode;
	CullMode cull_mode = CullMode::None;
	FrontFace front_face = FrontFace::Clockwise;
	Topology topology = Topology::TriangleList;
	std::vector<VertexBufferVK*> vertex_buffers; // TODO: store pointer and count, not std::vector
	IndexBufferVK* index_buffer = nullptr;
	std::optional<BlendMode> blendMode;

	bool pipelineStateDirty = true;
	bool scissorDirty = true;
	bool viewportDirty = true;
	bool depthModeDirty = true;
	bool stencilModeDirty = true;
	bool cullModeDirty = true;
	bool frontFaceDirty = true;
	bool topologyDirty = true;
	bool vertexBuffersDirty = true;
	bool indexBufferDirty = true;
	bool blendModeDirty = true;

	std::unordered_set<uint32_t> graphics_pipeline_ignore_bindings;

	uint32_t GetBackBufferWidth();
	uint32_t GetBackBufferHeight();
	vk::Format GetBackBufferFormat();

	bool render_pass_active = false;

	vk::PipelineStageFlags2 current_memory_stage = vk::PipelineStageFlagBits2::eTransfer;

	std::unordered_set<ObjectVK*> objects;
};

extern RenderContext gContext;

uint32_t GetMemoryType(vk::MemoryPropertyFlags properties, uint32_t type_bits);

std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(uint64_t size, vk::BufferUsageFlags usage);
vk::raii::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels = 1);
std::tuple<vk::raii::Image, vk::raii::DeviceMemory, vk::raii::ImageView> CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect_flags, uint32_t mip_levels = 1);

vk::DeviceAddress GetBufferDeviceAddress(vk::Buffer buffer);

void WriteToBuffer(vk::raii::DeviceMemory& memory, const void* data, size_t size);

void DestroyStaging(VulkanObject&& object);
void ReleaseStaging();

void BeginRenderPass();
void EndRenderPass();
void EnsureRenderPassActivated();
void EnsureRenderPassDeactivated();
void SetImageMemoryBarrier(const vk::raii::CommandBuffer& cmdbuf, vk::Image image, vk::ImageAspectFlags aspect_mask,
	vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t base_mip_level = 0,
	uint32_t level_count = VK_REMAINING_MIP_LEVELS, uint32_t base_array_layer = 0,
	uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS);

void SetImageMemoryBarrier(const vk::raii::CommandBuffer& cmdbuf, vk::Image image, vk::Format format,
	vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t base_mip_level = 0,
	uint32_t level_count = VK_REMAINING_MIP_LEVELS, uint32_t base_array_layer = 0,
	uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS);

void EnsureMemoryState(const vk::raii::CommandBuffer& cmdbuf, vk::PipelineStageFlags2 stage);
void OneTimeSubmit(std::function<void(const vk::raii::CommandBuffer&)> func);

std::tuple<vk::raii::PipelineLayout, vk::raii::DescriptorSetLayout, std::vector<vk::DescriptorSetLayoutBinding>> CreatePipelineLayout(const std::vector<std::vector<uint32_t>>& spirvs);

void PushDescriptorBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding, vk::DescriptorType type,
	const vk::raii::Buffer& buffer);
void PushDescriptorTexture(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding);
void PushDescriptorUniformBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding);
void PushDescriptorAccelerationStructure(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding);
void PushDescriptorStorageImage(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding);
void PushDescriptorStorageBuffer(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, uint32_t binding);
void PushDescriptors(vk::raii::CommandBuffer& cmdlist, vk::PipelineBindPoint pipeline_bind_point,
	const vk::raii::PipelineLayout& pipeline_layout, const std::vector<vk::DescriptorSetLayoutBinding>& required_descriptor_bindings,
	const std::unordered_set<uint32_t>& ignore_bindings = {});

RaytracingShaderBindingTable CreateRaytracingShaderBindingTable(const vk::raii::Pipeline& pipeline);

vk::raii::Pipeline CreateGraphicsPipeline(const PipelineStateVK& pipeline_state);
vk::raii::Pipeline CreateRaytracingPipeline(const RaytracingPipelineStateVK& pipeline_state);

void EnsureVertexBuffers(vk::raii::CommandBuffer& cmdlist);
void EnsureIndexBuffer(vk::raii::CommandBuffer& cmdlist);
void EnsureTopology(vk::raii::CommandBuffer& cmdlist);
void EnsureViewport(vk::raii::CommandBuffer& cmdlist);
void EnsureScissor(vk::raii::CommandBuffer& cmdlist);
void EnsureCullMode(vk::raii::CommandBuffer& cmdlist);
void EnsureFrontFace(vk::raii::CommandBuffer& cmdlist);
void EnsureBlendMode(vk::raii::CommandBuffer& cmdlist);
void EnsureDepthMode(vk::raii::CommandBuffer& cmdlist);
void EnsureStencilMode(vk::raii::CommandBuffer& cmdlist);
void EnsureGraphicsPipelineState(vk::raii::CommandBuffer& cmdlist);
void EnsureRaytracingPipelineState(vk::raii::CommandBuffer& cmdlist);
void EnsureGraphicsDescriptors(vk::raii::CommandBuffer& cmdlist);
void EnsureRaytracingDescriptors(vk::raii::CommandBuffer& cmdlist);
void EnsureGraphicsState(bool draw_indexed);
void EnsureRaytracingState();

void WaitForGpu();

void CreateSwapchain(uint32_t width, uint32_t height);

void MoveToNextFrame();
void RenderBegin();
void RenderEnd();

#endif // RENDER_VULKAN