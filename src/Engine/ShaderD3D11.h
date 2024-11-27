#pragma once

#if RENDER_D3D11

#include "ShaderCompiler.h"

SE_MAKE_HASHABLE(std::vector<InputLayout>, t);

class ShaderD3D11
{
public:
	ShaderD3D11(const std::string& vertex_code, const std::string& fragment_code, std::vector<std::string> defines);

	const auto& GetD3D11VertexShader() const { return mVertexShader; }
	const auto& GetD3D11PixelShader() const { return mPixelShader; }
	auto& GetInputLayoutCache() { return mInputLayoutCache; }
	const auto& GetVertexShaderBlob() const { return mVertexShaderBlob; }

private:
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	std::unordered_map<std::vector<InputLayout>, ComPtr<ID3D11InputLayout>> mInputLayoutCache;
	ComPtr<ID3DBlob> mVertexShaderBlob; // for input layout
};

#endif // RENDER_D3D11