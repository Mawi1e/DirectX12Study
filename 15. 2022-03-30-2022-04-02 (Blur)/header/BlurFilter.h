#pragma once

#include "VertexBuffer.h"

class BlurFilter {
public:
	BlurFilter();
	BlurFilter(ID3D12Device*, int, int, DXGI_FORMAT);
	BlurFilter(const BlurFilter&) = delete;
	BlurFilter& operator=(const BlurFilter&) = delete;
	~BlurFilter();

	void BuildDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE, CD3DX12_GPU_DESCRIPTOR_HANDLE, UINT);
	void ResizeBuffer(int, int);
	void Execute(ID3D12GraphicsCommandList*, ID3D12RootSignature*,
		ID3D12PipelineState*, ID3D12PipelineState*, ID3D12Resource*, int);
	ID3D12Resource* Output();

private:
	std::vector<float> GaussianBlur(float);
	void BuildResource();
	void BuildDescriptor();

	template <class _Tp>
	_Tp& unmove(_Tp&&);

private:
	ID3D12Device* m_Device;
	DXGI_FORMAT m_Format;
	int m_Width, m_Height;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_Blur0, m_Blur1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuSrv, m_Blur1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuUav, m_Blur1CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuSrv, m_Blur1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuUav, m_Blur1GpuUav;

	const static int m_MaxBlurRadius = 5;

};