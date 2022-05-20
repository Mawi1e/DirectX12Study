#include "CubeRenderTarget.h"

CubeRenderTarget::CubeRenderTarget(ID3D12Device* Device, UINT ScreenWidth, UINT ScreenHeight, DXGI_FORMAT RtvFormat) {
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;

	m_Viewport = { 0.0f, 0.0f, (float)ScreenWidth, (float)ScreenHeight, 0.0f, 1.0f };
	m_ScissorRect = { 0, 0, (int)ScreenWidth, (int)ScreenHeight };

	m_Device = Device;
	m_BackbufferFormat = RtvFormat;

	BuildResource();
}

CubeRenderTarget::~CubeRenderTarget() {
}

ID3D12Resource* CubeRenderTarget::Resource() const {
	return m_CubeMap.Get();
}

RECT CubeRenderTarget::GetScissorRect() const {
	return m_ScissorRect;
}

D3D12_VIEWPORT CubeRenderTarget::GetViewport() const {
	return m_Viewport;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CubeRenderTarget::SrvHandle() const {
	return m_GpuSrvHandle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CubeRenderTarget::RtvHandle(int rtvIndex) const {
	return m_CpuRtvHandle[rtvIndex];
}

void CubeRenderTarget::OnResize(UINT newWidth, UINT newHeight) {
	if ((m_ScreenWidth != newWidth) || (m_ScreenHeight != newHeight)) {

		m_ScreenWidth = newWidth;
		m_ScreenHeight = newHeight;

		BuildResource();
		BuildDescriptor();
	}
}

void CubeRenderTarget::BuildDescriptor(
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle[6]) {

	m_CpuSrvHandle = CpuSrvHandle;
	m_GpuSrvHandle = GpuSrvHandle;
	for (int i = 0; i < 6; ++i) {
		m_CpuRtvHandle[i] = CpuRtvHandle[i];
	}

	BuildDescriptor();
}

void CubeRenderTarget::BuildResource() {
	D3D12_RESOURCE_DESC rtvResourceDesc = {};
	rtvResourceDesc.Alignment = 0;
	rtvResourceDesc.DepthOrArraySize = 6;
	rtvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	rtvResourceDesc.Format = m_BackbufferFormat;
	rtvResourceDesc.Height = m_ScreenHeight;
	rtvResourceDesc.Width = m_ScreenWidth;
	rtvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvResourceDesc.MipLevels = 1;
	rtvResourceDesc.SampleDesc.Count = 1;
	rtvResourceDesc.SampleDesc.Quality = 0;

	m_Device->CreateCommittedResource(&unmove(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&rtvResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_CubeMap.GetAddressOf()));
}

void CubeRenderTarget::BuildDescriptor() {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_BackbufferFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	m_Device->CreateShaderResourceView(m_CubeMap.Get(), &srvDesc, m_CpuSrvHandle);

	for (int i = 0; i < 6; ++i) {
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = m_BackbufferFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		m_Device->CreateRenderTargetView(m_CubeMap.Get(), &rtvDesc, m_CpuRtvHandle[i]);
	}
}
