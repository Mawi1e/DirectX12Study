#include "ShadowMap.h"

ShadowMap::ShadowMap(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format) {
	m_Device = device;
	m_Width = width;
	m_Height = height;
	m_Format = format;

	m_Viewport = { 0.0f, 0.0f, (FLOAT)width, (FLOAT)height, 0.0f, 1.0f };
	m_Rect = { 0, 0, (LONG)width, (LONG)height };

	CreateResource();
}

ShadowMap::~ShadowMap() {
}

ID3D12Resource* ShadowMap::Resource() const {
	return m_ShadowMap.Get();
}

D3D12_VIEWPORT ShadowMap::Viewport() const {
	return m_Viewport;
}

D3D12_RECT ShadowMap::Rect() const {
	return m_Rect;
}

UINT ShadowMap::GetWidth() const {
	return m_Width;
}

UINT ShadowMap::GetHeight() const {
	return m_Height;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv() const {
	return m_CpuDsvHandle;
}

void ShadowMap::OnResize(UINT width, UINT height) {
	if (m_Width != width || m_Height != height) {
		m_Width = width;
		m_Height = height;

		CreateResource();

		CreateDescriptor();
	}
}

void ShadowMap::CreateResource() {
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	resourceDesc.Height = m_Height;
	resourceDesc.Width = m_Width;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.DepthStencil.Depth = 1.0f;
	clearVal.DepthStencil.Stencil = 0;
	clearVal.Format = m_Format;

	m_Device->CreateCommittedResource(
		&sh_unmove(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearVal,
		IID_PPV_ARGS(m_ShadowMap.GetAddressOf()));
}

void ShadowMap::CreateDescriptor() {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_Device->CreateShaderResourceView(m_ShadowMap.Get(), &srvDesc, m_CpuSrvHandle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = m_Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_Device->CreateDepthStencilView(m_ShadowMap.Get(), &dsvDesc, m_CpuDsvHandle);
}

void ShadowMap::CreateDescriptor(
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle) {

	m_CpuSrvHandle = srvCpuHandle;
	m_GpuSrvHandle = srvGpuHandle;
	m_CpuDsvHandle = dsvCpuHandle;

	CreateDescriptor();
}









