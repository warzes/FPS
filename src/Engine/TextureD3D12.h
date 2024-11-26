#pragma once

#if RENDER_D3D12

#include "RenderCoreD3D12.h"

class TextureD3D12 final
{
public:
	TextureD3D12(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count);
	TextureD3D12(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D12Resource> texture);
	~TextureD3D12();

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel, uint32_t offset_x, uint32_t offset_y);

	void GenerateMips(ID3D12GraphicsCommandList* cmdlist, std::vector<ComPtr<ID3D12DeviceChild>>& stagingObjects);
	void GenerateMips();

	void EnsureState(ID3D12GraphicsCommandList* cmdlist, D3D12_RESOURCE_STATES state);

	const auto& GetD3D12Texture() const { return m_texture; }
	auto GetGpuDescriptorHandle() const { return m_gpuDescriptorHandle; }
	auto GetWidth() const { return m_width; }
	auto GetHeight() const { return m_height; }
	auto GetFormat() const { return m_format; }

private:
	ComPtr<ID3D12Resource> m_texture;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuDescriptorHandle;
	D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_mipCount = 0;
	PixelFormat m_format;
};

#endif // RENDER_D3D12