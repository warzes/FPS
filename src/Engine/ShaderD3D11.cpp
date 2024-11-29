#include "stdafx.h"
#if RENDER_D3D11
#include "ShaderD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
ShaderD3D11::ShaderD3D11(const std::string& vertexCode, const std::string& fragmentCode, std::vector<std::string> defines)
{
	auto vertexShaderSpirv   = CompileGlslToSpirv(ShaderStage::Vertex, vertexCode, defines);
	auto fragmentShaderSpirv = CompileGlslToSpirv(ShaderStage::Fragment, fragmentCode, defines);

	auto hlslVert = CompileSpirvToHlsl(vertexShaderSpirv, 40);
	auto hlslFrag = CompileSpirvToHlsl(fragmentShaderSpirv, 40);

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	ComPtr<ID3DBlob> vertexShaderError;
	if (FAILED(D3DCompile(hlslVert.c_str(), hlslVert.size(), nullptr, nullptr, nullptr, "main", "vs_4_0", flags, 0, m_vertexShaderBlob.GetAddressOf(), vertexShaderError.GetAddressOf())))
	{
		std::string shaderErrorString = "";
		if (vertexShaderError)
			shaderErrorString = std::string((char*)vertexShaderError->GetBufferPointer(), vertexShaderError->GetBufferSize());

		Fatal("D3DCompile() failed: " + shaderErrorString);
		return;
	}

	ComPtr<ID3DBlob> pixelShaderError;
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (FAILED(D3DCompile(hlslFrag.c_str(), hlslFrag.size(), nullptr, nullptr, nullptr, "main", "ps_4_0", flags, 0, pixelShaderBlob.GetAddressOf(), pixelShaderError.GetAddressOf())))
	{
		std::string shaderErrorString = "";
		if (pixelShaderError)
			shaderErrorString = std::string((char*)pixelShaderError->GetBufferPointer(), pixelShaderError->GetBufferSize());
		Fatal("D3DCompile() failed: " + shaderErrorString);
		return;
	}
	
	if (FAILED(gContext.device->CreateVertexShader(m_vertexShaderBlob->GetBufferPointer(), m_vertexShaderBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf())))
	{
		Fatal("CreateVertexShader() failed");
		return;
	}

	if (FAILED(gContext.device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf())))
	{
		Fatal("CreatePixelShader() failed");
		return;
	}
}
//=============================================================================
#endif // RENDER_D3D11