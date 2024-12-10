#include "stdafx.h"
#if RENDER_D3D12
#include "ShaderD3D12.h"
#include "ContextD3D12.h"
//=============================================================================
ShaderD3D12::ShaderD3D12(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines)
{
	auto vertex_shader_spirv = CompileGlslToSpirv(ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = CompileGlslToSpirv(ShaderStage::Fragment, fragment_code, defines);

	auto hlsl_vert = CompileSpirvToHlsl(vertex_shader_spirv, 50);
	auto hlsl_frag = CompileSpirvToHlsl(fragment_shader_spirv, 50);

	ComPtr<ID3DBlob> vertex_shader_error;
	ComPtr<ID3DBlob> pixel_shader_error;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flags = 0;
#endif
	D3DCompile(hlsl_vert.c_str(), hlsl_vert.size(), NULL, NULL, NULL, "main", "vs_5_0", compile_flags, 0,
		&m_vertexShaderBlob, &vertex_shader_error);
	D3DCompile(hlsl_frag.c_str(), hlsl_frag.size(), NULL, NULL, NULL, "main", "ps_5_0", compile_flags, 0,
		&m_pixelShaderBlob, &pixel_shader_error);

	std::string vertex_shader_error_string = "";
	std::string pixel_shader_error_string = "";

	if (vertex_shader_error != NULL)
		vertex_shader_error_string = std::string((char*)vertex_shader_error->GetBufferPointer(), vertex_shader_error->GetBufferSize());

	if (pixel_shader_error != NULL)
		pixel_shader_error_string = std::string((char*)pixel_shader_error->GetBufferPointer(), pixel_shader_error->GetBufferSize());

	if (m_vertexShaderBlob == NULL)
		throw std::runtime_error(vertex_shader_error_string);

	if (m_pixelShaderBlob == NULL)
		throw std::runtime_error(pixel_shader_error_string);

	auto vertex_shader_reflection = MakeSpirvReflection(vertex_shader_spirv);
	auto fragment_shader_reflection = MakeSpirvReflection(fragment_shader_spirv);

	for (const auto& reflection : { vertex_shader_reflection, fragment_shader_reflection })
	{
		for (const auto& [type, descriptor_bindings] : reflection.typedDescriptorBindings)
		{
			for (const auto& [binding, descriptor] : descriptor_bindings)
			{
				auto& required_descriptor_bindings = m_requiredTypedDescriptorBindings[type];
				if (required_descriptor_bindings.contains(binding))
					continue;

				required_descriptor_bindings[binding] = descriptor;
			}
		}
		for (const auto& [set, bindings] : reflection.descriptorSets)
		{
			m_requiredDescriptorSets[reflection.stage][set] = bindings;
		}
	}

	{
		std::vector<CD3DX12_ROOT_PARAMETER> params;
		std::vector<D3D12_STATIC_SAMPLER_DESC> static_samplers;
		std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges(32);

		for (const auto& [type, descriptor_bindings] : m_requiredTypedDescriptorBindings)
		{
			for (const auto& [binding, descriptor] : descriptor_bindings)
			{
				CD3DX12_ROOT_PARAMETER param;

				if (type == ShaderReflection::DescriptorType::UniformBuffer)
				{
					param.InitAsConstantBufferView(binding);
				}
				else if (type == ShaderReflection::DescriptorType::CombinedImageSampler)
				{
					auto range = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, binding);
					ranges.push_back(range);

					param.InitAsDescriptorTable(1, &ranges[ranges.size() - 1], D3D12_SHADER_VISIBILITY_ALL);

					static_samplers.push_back(CD3DX12_STATIC_SAMPLER_DESC(binding, D3D12_FILTER_MIN_MAG_MIP_LINEAR));
				}
				else
				{
					assert(false);
				}

				m_bindingToRootIndexMap.insert({ binding, (uint32_t)params.size() });
				params.push_back(param);
			}
		}

		auto desc = CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC((UINT)params.size(), params.data(), (UINT)static_samplers.size(),
			static_samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		DirectX::CreateRootSignature(gContext.device.Get(), &desc.Desc_1_0, m_rootSignature.GetAddressOf());
	}
}

ShaderD3D12::~ShaderD3D12()
{
	DestroyStaging(m_rootSignature);
}
#endif // RENDER_D3D12