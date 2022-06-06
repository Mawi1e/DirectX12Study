#pragma once
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxcompiler")

#include <wrl.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <d3d12shader.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

using namespace DirectX;

template <class _Tp>
_Tp& sh_unmove(_Tp&& __value) {
	return __value;
}

class ShadowMap {
public:
	ShadowMap(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format);
	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator=(const ShadowMap&) = delete;
	virtual ~ShadowMap();

	ID3D12Resource* Resource() const;
	D3D12_VIEWPORT Viewport() const;
	D3D12_RECT Rect() const;
	UINT GetWidth() const;
	UINT GetHeight() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv() const;

	void OnResize(UINT width, UINT height);
	void CreateResource();
	void CreateDescriptor();
	void CreateDescriptor(
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle
	);

private:
	ID3D12Device* m_Device = nullptr;
	UINT m_Width, m_Height;
	DXGI_FORMAT m_Format;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Rect;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle, m_CpuDsvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;

};