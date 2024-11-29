#pragma once

#if RENDER_D3D11

#include "ShaderCompiler.h"

class ShaderD3D11 final
{
public:
	ShaderD3D11() = delete;
	ShaderD3D11(const std::string& vertexCode, const std::string& fragmentCode, std::vector<std::string> defines);

	const auto& GetD3D11VertexShader() const { return m_vertexShader; }
	const auto& GetD3D11PixelShader() const { return m_pixelShader; }
	auto& GetInputLayoutCache() { return m_inputLayoutCache; }
	const auto& GetVertexShaderBlob() const { return m_vertexShaderBlob; }

private:
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader>  m_pixelShader;
	std::unordered_map<std::vector<InputLayout>, ComPtr<ID3D11InputLayout>> m_inputLayoutCache;
	ComPtr<ID3DBlob>           m_vertexShaderBlob; // for input layout
};

#endif // RENDER_D3D11