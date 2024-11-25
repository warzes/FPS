#include "stdafx.h"
#include "RenderSystem.h"
#include "ContextD3D12.h"
#if RENDER_D3D12
//=============================================================================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RenderSystem::createAPI(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo)
{
	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
}
//=============================================================================
#endif // RENDER_D3D12