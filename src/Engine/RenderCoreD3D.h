#pragma once

#if RENDER_D3D11 || RENDER_D3D12

const std::string DXErrorToStr(HRESULT hr);

#endif // RENDER_D3D11 || RENDER_D3D12