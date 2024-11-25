#pragma once

#if RENDER_D3D12

#include "ShaderCompiler.h"

class ShaderD3D12 final
{
public:
	ShaderD3D12(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines);
	~ShaderD3D12();

	const auto& GetRootSignature() const { return m_rootSignature; }
	const auto& GetRequiredTypedDescriptorBindings() const { return m_requiredTypedDescriptorBindings; }
	const auto& GetBindingToRootIndexMap() const { return m_bindingToRootIndexMap; }
	const auto& GetVertexShaderBlob() const { return m_vertexShaderBlob; }
	const auto& GetPixelShaderBlob() const { return m_pixelShaderBlob; }

private:
	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::unordered_map<ShaderReflection::DescriptorType, std::unordered_map<uint32_t, ShaderReflection::Descriptor>> m_requiredTypedDescriptorBindings;
	std::unordered_map<ShaderStage, std::unordered_map<uint32_t/*set*/, std::unordered_set<uint32_t>/*bindings*/>> m_requiredDescriptorSets;
	std::unordered_map<uint32_t, uint32_t> m_bindingToRootIndexMap;
	ComPtr<ID3DBlob> m_vertexShaderBlob;
	ComPtr<ID3DBlob> m_pixelShaderBlob;
};

#endif // RENDER_D3D12