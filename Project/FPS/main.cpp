#include "stdafx.h"

#if defined(_MSC_VER)
#	pragma comment( lib, "Engine.lib" )
#	pragma comment( lib, "3rdparty.lib" )
#endif

bool IsRequestExit = false;
void RequestExit()
{
	IsRequestExit = true;
}

int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	LogSystem log;
	log.Create({});

	WindowSystem window;
	window.Create({});

	RenderSystem render;
	render.Create(window, {});

	// Create Vertex Shader
	ID3DBlob* vsBlob;
	ID3D11VertexShader* vertexShader;
	{
		ID3DBlob* shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char* errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (shaderCompileErrorsBlob) {
				errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			return 1;
		}

		hResult = render.GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
		assert(SUCCEEDED(hResult));
	}

	// Create Pixel Shader
	ID3D11PixelShader* pixelShader;
	{
		ID3DBlob* psBlob;
		ID3DBlob* shaderCompileErrorsBlob;
		HRESULT hResult = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorsBlob);
		if (FAILED(hResult))
		{
			const char* errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (shaderCompileErrorsBlob) {
				errorString = (const char*)shaderCompileErrorsBlob->GetBufferPointer();
				shaderCompileErrorsBlob->Release();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			return 1;
		}

		hResult = render.GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
		assert(SUCCEEDED(hResult));
		psBlob->Release();
	}

	// Create Input Layout
	ID3D11InputLayout* inputLayout;
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HRESULT hResult = render.GetDevice()->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
		assert(SUCCEEDED(hResult));
		vsBlob->Release();
	}

	// Create Vertex Buffer
	ID3D11Buffer* vertexBuffer;
	UINT numVerts;
	UINT stride;
	UINT offset;
	{
		float vertexData[] = { // x, y, u, v
			-0.5f,  0.5f, 0.f, 0.f,
			0.5f, -0.5f, 1.f, 1.f,
			-0.5f, -0.5f, 0.f, 1.f,
			-0.5f,  0.5f, 0.f, 0.f,
			0.5f,  0.5f, 1.f, 0.f,
			0.5f, -0.5f, 1.f, 1.f
		};
		stride = 4 * sizeof(float);
		numVerts = sizeof(vertexData) / stride;
		offset = 0;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(vertexData);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceData = { vertexData };

		HRESULT hResult = render.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
		assert(SUCCEEDED(hResult));
	}

	// Create Constant Buffer
	struct Constants
	{
		float pos[2];
		float paddingUnused[2]; // color (below) needs to be 16-byte aligned! 
		float color[4];
	};

	ID3D11Buffer* constantBuffer;
	{
		D3D11_BUFFER_DESC constantBufferDesc = {};
		// ByteWidth must be a multiple of 16, per the docs
		constantBufferDesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hResult = render.GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
		assert(SUCCEEDED(hResult));
	}

	// Create Sampler State
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	ID3D11SamplerState* samplerState;
	HRESULT hResult = render.GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
	assert(SUCCEEDED(hResult));

	// Load Image
	int texWidth, texHeight, texNumChannels;
	int texForceNumChannels = 4;
	unsigned char* testTextureBytes = stbi_load("testTexture.png", &texWidth, &texHeight, &texNumChannels, texForceNumChannels);
	assert(testTextureBytes);
	int texBytesPerRow = 4 * texWidth;

	// Create Texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = texWidth;
	textureDesc.Height = texHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
	textureSubresourceData.pSysMem = testTextureBytes;
	textureSubresourceData.SysMemPitch = texBytesPerRow;

	ID3D11Texture2D* texture;
	hResult = render.GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);
	assert(SUCCEEDED(hResult));

	ID3D11ShaderResourceView* textureView;
	hResult = render.GetDevice()->CreateShaderResourceView(texture, nullptr, &textureView);
	assert(SUCCEEDED(hResult));

	stbi_image_free(testTextureBytes);

	// Timing
	LONGLONG startPerfCount = 0;
	LONGLONG perfCounterFrequency = 0;
	{
		LARGE_INTEGER perfCount;
		QueryPerformanceCounter(&perfCount);
		startPerfCount = perfCount.QuadPart;
		LARGE_INTEGER perfFreq;
		QueryPerformanceFrequency(&perfFreq);
		perfCounterFrequency = perfFreq.QuadPart;
	}
	double currentTimeInSeconds = 0.0;

	FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };

	while (!IsRequestExit)
	{
		window.PollEvent();

		float dt;
		{
			double previousTimeInSeconds = currentTimeInSeconds;
			LARGE_INTEGER perfCount;
			QueryPerformanceCounter(&perfCount);

			currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
			dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
			if (dt > (1.f / 60.f))
				dt = (1.f / 60.f);
		}

		// Modulate player's y-position
		float playerPosY = {};
		const float posCycleAmplitude = 0.5f;
		const float posCyclePeriod = 3.f; // in seconds
		const float posCycleFreq = 2 * M_PI / posCyclePeriod;
		playerPosY = posCycleAmplitude * sinf(posCycleFreq * (float)currentTimeInSeconds);

		// Cycle player color
		float playerColor[4];
		const float colorCyclePeriod = 5.f; //in seconds
		const float colorCycleFreq = 2 * M_PI / colorCyclePeriod;
		playerColor[0] = 0.5f * (sinf(colorCycleFreq * (float)currentTimeInSeconds) + 1);
		playerColor[1] = 1 - playerPosY;
		playerColor[2] = 0.f;
		playerColor[3] = 1.f;

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		render.GetDeviceContext()->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		Constants* constants = (Constants*)(mappedSubresource.pData);
		constants->pos[0] = 0.0f;
		constants->pos[1] = playerPosY;
		constants->color[0] = playerColor[0];
		constants->color[1] = playerColor[1];
		constants->color[2] = playerColor[2];
		constants->color[3] = playerColor[3];
		render.GetDeviceContext()->Unmap(constantBuffer, 0);

		D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(window.GetWindowWidth()), (FLOAT)(window.GetWindowHeight()), 0.0f, 1.0f };
		auto* rtv = render.GetRenderTargetView();
		auto dsv = render.GetDepthStencilView();
		
		render.GetDeviceContext()->ClearRenderTargetView(rtv, backgroundColor);

		render.GetDeviceContext()->RSSetViewports(1, &viewport);

		render.GetDeviceContext()->OMSetRenderTargets(1, &rtv, nullptr);

		render.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		render.GetDeviceContext()->IASetInputLayout(inputLayout);

		render.GetDeviceContext()->VSSetShader(vertexShader, nullptr, 0);
		render.GetDeviceContext()->PSSetShader(pixelShader, nullptr, 0);

		render.GetDeviceContext()->PSSetShaderResources(0, 1, &textureView);
		render.GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);

		render.GetDeviceContext()->VSSetConstantBuffers(0, 1, &constantBuffer);

		render.GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

		render.GetDeviceContext()->Draw(numVerts, 0);

		render.Present();
	}

	render.Destroy();
	window.Destroy();
	log.Destroy();
}