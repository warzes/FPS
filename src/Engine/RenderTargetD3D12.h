#pragma once

#if RENDER_D3D12

#include "RenderCoreD3D12.h"

class RenderTargetD3D12 final
{
public:
	RenderTargetD3D12(uint32_t width, uint32_t height, TextureD3D12* texture, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor);
	RenderTargetD3D12(uint32_t width, uint32_t height, TextureD3D12* texture);
	~RenderTargetD3D12();

	const auto& GetRtvHeap() const { return m_rtvHeap; }
	const auto& GetDsvHeap() const { return m_dsvHeap; }
	const auto& GetDepthStencilResource() const { return m_depthStencilResource; }
	auto GetTexture() const { return m_texture; }
	auto GetDepthStencilFormat() const { return MainRenderTargetDepthStencilAttachmentFormat; }

private:
	void create(uint32_t width, uint32_t height, ComPtr<ID3D12Resource> textureResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor);

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12Resource>       m_depthStencilResource;
	TextureD3D12*                m_texture;
};

#endif // RENDER_D3D12
