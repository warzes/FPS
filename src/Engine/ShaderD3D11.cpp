#include "stdafx.h"
#if RENDER_D3D11
#include "ShaderD3D11.h"
#include "ContextD3D11.h"
//=============================================================================
ShaderD3D11::ShaderD3D11(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines)
{
	ComPtr<ID3DBlob> pixel_shader_blob;

	ComPtr<ID3DBlob> vertex_shader_error;
	ComPtr<ID3DBlob> pixel_shader_error;

	auto vertex_shader_spirv = CompileGlslToSpirv(ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = CompileGlslToSpirv(ShaderStage::Fragment, fragment_code, defines);

	auto hlsl_vert = CompileSpirvToHlsl(vertex_shader_spirv, 40);
	auto hlsl_frag = CompileSpirvToHlsl(fragment_shader_spirv, 40);

	D3DCompile(hlsl_vert.c_str(), hlsl_vert.size(), NULL, NULL, NULL, "main", "vs_4_0", 0, 0,
		mVertexShaderBlob.GetAddressOf(), vertex_shader_error.GetAddressOf());

	D3DCompile(hlsl_frag.c_str(), hlsl_frag.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0,
		pixel_shader_blob.GetAddressOf(), pixel_shader_error.GetAddressOf());

	std::string vertex_shader_error_string = "";
	std::string pixel_shader_error_string = "";

	if (vertex_shader_error != NULL)
		vertex_shader_error_string = std::string((char*)vertex_shader_error->GetBufferPointer(), vertex_shader_error->GetBufferSize());

	if (pixel_shader_error != NULL)
		pixel_shader_error_string = std::string((char*)pixel_shader_error->GetBufferPointer(), pixel_shader_error->GetBufferSize());

	if (mVertexShaderBlob == NULL)
		throw std::runtime_error(vertex_shader_error_string);

	if (pixel_shader_blob == NULL)
		throw std::runtime_error(pixel_shader_error_string);

	gContext.device->CreateVertexShader(mVertexShaderBlob->GetBufferPointer(), mVertexShaderBlob->GetBufferSize(),
		NULL, mVertexShader.GetAddressOf());

	gContext.device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(),
		NULL, mPixelShader.GetAddressOf());
}
//=============================================================================
#endif // RENDER_D3D11