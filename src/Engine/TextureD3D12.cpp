#include "stdafx.h"
#if RENDER_D3D12
#include "TextureD3D12.h"
#include "ContextD3D12.h"
//=============================================================================
TextureD3D12::TextureD3D12(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
	: m_width(width)
	, m_height(height)
	, m_mipCount(mip_count)
	, m_format(format)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(PixelFormatMap.at(m_format), width, height, 1, (UINT16)mip_count);

	desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	gContext.device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(m_texture.GetAddressOf()));

	DirectX::CreateShaderResourceView(gContext.device.Get(), m_texture.Get(), gContext.descriptorHeapCPUHandle);

	m_gpuDescriptorHandle = gContext.descriptorHeapGPUHandle;

	gContext.descriptorHeapCPUHandle.Offset(1, gContext.descriptorHandleIncrementSize);
	gContext.descriptorHeapGPUHandle.Offset(1, gContext.descriptorHandleIncrementSize);
}
//=============================================================================
TextureD3D12::TextureD3D12(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D12Resource> texture)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_texture(texture)
{
}
//=============================================================================
TextureD3D12::~TextureD3D12()
{
	DestroyStaging(m_texture);
}
//=============================================================================
void TextureD3D12::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto upload_size = GetRequiredIntermediateSize(m_texture.Get(), mip_level, 1);
	auto upload_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_size);
	auto upload_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ComPtr<ID3D12Resource> upload_buffer = NULL;

	gContext.device->CreateCommittedResource(&upload_prop, D3D12_HEAP_FLAG_NONE, &upload_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(upload_buffer.GetAddressOf()));

	auto channels = GetFormatChannelsCount(format);
	auto channel_size = GetFormatChannelSize(format);

	D3D12_SUBRESOURCE_DATA subersource_data = {};
	subersource_data.pData = memory;
	subersource_data.RowPitch = width * channels * channel_size;
	subersource_data.SlicePitch = width * height * channels * channel_size;

	OneTimeSubmit([&](ID3D12GraphicsCommandList* cmdlist) {
		EnsureState(cmdlist, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources(cmdlist, m_texture.Get(), upload_buffer.Get(), 0, mip_level, 1, &subersource_data);
		});
}
//=============================================================================
void TextureD3D12::GenerateMips(ID3D12GraphicsCommandList* cmdlist, std::vector<ComPtr<ID3D12DeviceChild>>& staging_objects)
{
	EnsureState(cmdlist, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	D3D12GenerateMips(gContext.device.Get(), cmdlist, m_texture.Get(), staging_objects);
	m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}
//=============================================================================
void TextureD3D12::GenerateMips()
{
	OneTimeSubmit([&](ID3D12GraphicsCommandList* cmdlist) {
		GenerateMips(cmdlist, gContext.stagingObjects);
		});
}
//=============================================================================
void TextureD3D12::EnsureState(ID3D12GraphicsCommandList* cmdlist, D3D12_RESOURCE_STATES state)
{
	if (m_currentState == state) return;

	DirectX::TransitionResource(cmdlist, m_texture.Get(), m_currentState, state);
	m_currentState = state;
}
//=============================================================================
#endif // RENDER_D3D12