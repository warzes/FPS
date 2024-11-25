#include "stdafx.h"
#if RENDER_D3D12
#include "RenderTargetD3D12.h"
#include "TextureD3D12.h"
#include "ContextD3D12.h"
//=============================================================================
RenderTargetD3D12::RenderTargetD3D12(uint32_t width, uint32_t height, TextureD3D12* texture, D3D12_CPU_DESCRIPTOR_HANDLE rtv_descriptor) : m_texture(texture)
{
	create(width, height, m_texture->GetD3D12Texture(), rtv_descriptor);
}
//=============================================================================
RenderTargetD3D12::RenderTargetD3D12(uint32_t width, uint32_t height, TextureD3D12* texture) : m_texture(texture)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.NumDescriptors = 1;
	gContext.device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf()));

	auto rtv_descriptor = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	create(width, height, m_texture->GetD3D12Texture(), rtv_descriptor);
}
//=============================================================================
RenderTargetD3D12::~RenderTargetD3D12()
{
	DestroyStaging(m_rtvHeap);
	DestroyStaging(m_dsvHeap);
	DestroyStaging(m_depthStencilResource);
}
//=============================================================================
void RenderTargetD3D12::create(uint32_t width, uint32_t height, ComPtr<ID3D12Resource> texture_resource,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_descriptor)
{
	DirectX::CreateRenderTargetView(gContext.device.Get(), texture_resource.Get(), rtv_descriptor);

	auto depth_heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto depth_desc = CD3DX12_RESOURCE_DESC::Tex2D(GetDepthStencilFormat(),
		(UINT64)width, (UINT)height, 1, 1);

	depth_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	gContext.device->CreateCommittedResource(&depth_heap_props, D3D12_HEAP_FLAG_NONE, &depth_desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, NULL, IID_PPV_ARGS(m_depthStencilResource.GetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.NumDescriptors = 1;
	gContext.device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf()));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.Format = depth_desc.Format;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	gContext.device->CreateDepthStencilView(m_depthStencilResource.Get(), &dsv_desc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
//=============================================================================
#endif // RENDER_D3D12