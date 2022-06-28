#pragma once

#include "VertexBuffer.h"

template <class _Tp> _Tp& unmove(_Tp&& __val) { return __val; }

class CubeRenderTarget {
public:
	CubeRenderTarget(ID3D12Device* Device, UINT ScreenWidth, UINT ScreenHeight, DXGI_FORMAT RtvFormat);
	CubeRenderTarget(const CubeRenderTarget&) = delete;
	CubeRenderTarget& operator=(const CubeRenderTarget&) = delete;
	~CubeRenderTarget();

	ID3D12Resource* Resource() const;

	RECT GetScissorRect() const;
	D3D12_VIEWPORT GetViewport() const;

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvHandle() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(int rtvIndex) const;

	void OnResize(UINT newWidth, UINT newHeight);

	void BuildDescriptor(
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6]);

private:
	void BuildResource();
	void BuildDescriptor();

private:
	ID3D12Device* m_Device;

	D3D12_RECT m_ScissorRect;
	D3D12_VIEWPORT m_Viewport;

	UINT m_ScreenWidth, m_ScreenHeight;
	DXGI_FORMAT m_BackbufferFormat;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuRtvHandle[6];

	Microsoft::WRL::ComPtr<ID3D12Resource> m_CubeMap;

};