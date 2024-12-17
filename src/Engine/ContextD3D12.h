#pragma once

https://github.com/Delt06/dx12-renderer
https://www.3dgep.com/learning-directx-12-1/
#if RENDER_D3D12

#include "RenderCoreD3D12.h"
#include "PipelineStateD3D12.h"

class RenderContext final
{
public:
	ComPtr<ID3D12Device>               device;
	ComPtr<ID3D12CommandAllocator>     commandAllocator;
	ComPtr<ID3D12CommandQueue>         commandQueue;
	ComPtr<IDXGISwapChain3>            swapChain;
	ComPtr<ID3D12GraphicsCommandList4> commandList;
	ComPtr<ID3D12DescriptorHeap>       descriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE      descriptorHeapCPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE      descriptorHeapGPUHandle;
	UINT                               descriptorHandleIncrementSize{ 0 };

	struct Frame
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor; // TODO: move inside RenderTargetD3D12
		TextureD3D12*               backBufferTexture;
		RenderTargetD3D12*          mainRenderTarget;
	};
	ComPtr<ID3D12DescriptorHeap>        frameRTVHeap;

	Frame                               frames[RHI_BACKBUFFER_COUNT];
	UINT                                frameIndex{ 0 };

	HANDLE                              fenceEvent{ nullptr };
	ComPtr<ID3D12Fence>                 fence;
	UINT64                              fenceValue;

	Frame& GetCurrentFrame() { return frames[frameIndex]; }

	std::unordered_map<uint32_t, TextureD3D12*>       textures;
	std::unordered_map<uint32_t, UniformBufferD3D12*> uniformBuffers;
	std::vector<RenderTargetD3D12*>                   renderTargets;

	PipelineStateD3D12                                                  pipelineState;
	std::unordered_map<PipelineStateD3D12, ComPtr<ID3D12PipelineState>> pipelineStates;

	Topology                        topology = Topology::TriangleList;
	std::optional<Viewport>         viewport;
	std::optional<Scissor>          scissor;
	std::vector<VertexBufferD3D12*> vertexBuffers;
	IndexBufferD3D12*               indexBuffer = nullptr;

	bool topologyDirty = true;
	bool viewportDirty = true;
	bool scissorDirty = true;
	bool vertexBuffersDirty = true;
	bool indexBufferDirty = true;

	uint32_t width = 0;
	uint32_t height = 0;

	std::vector<ComPtr<ID3D12DeviceChild>> stagingObjects;

	uint32_t    GetBackBufferWidth();
	uint32_t    GetBackBufferHeight();
	PixelFormat GetBackBufferFormat();
private:
};

extern RenderContext gContext;

void DestroyStaging(ComPtr<ID3D12DeviceChild> object);
void ReleaseStaging();

void BeginCommandList(ID3D12CommandAllocator* cmd_alloc, ID3D12GraphicsCommandList* cmd_list);
void EndCommandList(ID3D12CommandQueue* cmd_queue, ID3D12GraphicsCommandList* cmd_list, bool wait_for);
void OneTimeSubmit(std::function<void(ID3D12GraphicsCommandList*)> func);

ComPtr<ID3D12Resource> CreateBuffer(uint64_t size);

void WaitForGpu();

void CreateMainRenderTarget(uint32_t width, uint32_t height);
void DestroyMainRenderTarget();

void EnsureViewport();
void EnsureScissor();
void EnsureTopology();
void EnsureIndexBuffer();
void EnsureVertexBuffer();
void EnsureGraphicsState(bool draw_indexed);

void RenderBegin(); // TODO:
void RenderEnd(); // TODO:

#endif // RENDER_D3D12