﻿#include "stdafx.h"
#if RENDER_D3D11 || RENDER_D3D12
#include "RenderCoreD3D.h"
//=============================================================================
const std::string DXErrorToStr(HRESULT hr)
{
#define CASE_TO_STR(VALUE) case VALUE: return std::string(#VALUE)

	switch (hr)
	{
		// see https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
		CASE_TO_STR(S_OK);
		CASE_TO_STR(S_FALSE);
		CASE_TO_STR(E_ABORT);
		CASE_TO_STR(E_ACCESSDENIED);
		CASE_TO_STR(E_FAIL);
		CASE_TO_STR(E_HANDLE);
		CASE_TO_STR(E_INVALIDARG);
		CASE_TO_STR(E_NOINTERFACE);
		CASE_TO_STR(E_NOTIMPL);
		CASE_TO_STR(E_OUTOFMEMORY);
		CASE_TO_STR(E_POINTER);
		CASE_TO_STR(E_UNEXPECTED);

		// see https://msdn.microsoft.com/en-us/library/windows/desktop/bb509553(v=vs.85).aspx
		CASE_TO_STR(DXGI_ERROR_DEVICE_HUNG);
		CASE_TO_STR(DXGI_ERROR_DEVICE_REMOVED);
		CASE_TO_STR(DXGI_ERROR_DEVICE_RESET);
		CASE_TO_STR(DXGI_ERROR_DRIVER_INTERNAL_ERROR);
		CASE_TO_STR(DXGI_ERROR_FRAME_STATISTICS_DISJOINT);
		CASE_TO_STR(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE);
		CASE_TO_STR(DXGI_ERROR_INVALID_CALL);
		CASE_TO_STR(DXGI_ERROR_MORE_DATA);
		CASE_TO_STR(DXGI_ERROR_NONEXCLUSIVE);
		CASE_TO_STR(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE);
		CASE_TO_STR(DXGI_ERROR_NOT_FOUND);
		CASE_TO_STR(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED);
		CASE_TO_STR(DXGI_ERROR_REMOTE_OUTOFMEMORY);
		CASE_TO_STR(DXGI_ERROR_WAS_STILL_DRAWING);
		CASE_TO_STR(DXGI_ERROR_UNSUPPORTED);
		CASE_TO_STR(DXGI_ERROR_ACCESS_LOST);
		CASE_TO_STR(DXGI_ERROR_WAIT_TIMEOUT);
		CASE_TO_STR(DXGI_ERROR_SESSION_DISCONNECTED);
		CASE_TO_STR(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE);
		CASE_TO_STR(DXGI_ERROR_CANNOT_PROTECT_CONTENT);
		CASE_TO_STR(DXGI_ERROR_ACCESS_DENIED);
		CASE_TO_STR(DXGI_ERROR_NAME_ALREADY_EXISTS);
		CASE_TO_STR(DXGI_ERROR_SDK_COMPONENT_MISSING);

		// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476174(v=vs.85).aspx
		CASE_TO_STR(D3D10_ERROR_FILE_NOT_FOUND);
		CASE_TO_STR(D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);

		CASE_TO_STR(D3D11_ERROR_FILE_NOT_FOUND);
		CASE_TO_STR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
		CASE_TO_STR(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS);
		CASE_TO_STR(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD);

		CASE_TO_STR(D3D12_ERROR_ADAPTER_NOT_FOUND);
		CASE_TO_STR(D3D12_ERROR_DRIVER_VERSION_MISMATCH);

		// TODO: остальное
	}
	return "";
#undef CASE_TO_STR
}
//=============================================================================
#endif // RENDER_D3D11 || RENDER_D3D12