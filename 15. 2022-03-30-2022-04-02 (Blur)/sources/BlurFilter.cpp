#include "BlurFilter.h"

BlurFilter::BlurFilter() {
}

BlurFilter::BlurFilter(ID3D12Device* d3dDevice, int width, int height, DXGI_FORMAT format) {
	m_Device = d3dDevice;
	m_Format = format;
	m_Width = width;
	m_Height = height;

	BuildResource();
}

BlurFilter::~BlurFilter() {
}

std::vector<float> BlurFilter::GaussianBlur(float sigma) {
	float twoSigma2 = 2.0f * sigma * sigma;
	int blurRadius = (int)sigma * 2;

	assert(blurRadius <= m_MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i) {
		float x = (float)i;

		weights[i + blurRadius] = exp(-x * x / twoSigma2);
		weightSum += weights[i + blurRadius];
	}

	for (size_t i = 0; i < weights.size(); ++i) {
		weights[i] /= weightSum;
	}

	return weights;
}

void BlurFilter::BuildResource() {
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resourceDesc.Format = m_Format;
	resourceDesc.Height = m_Height;
	resourceDesc.Width = m_Width;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	m_Device->CreateCommittedResource(&unmove(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_Blur0.GetAddressOf()));

	m_Device->CreateCommittedResource(&unmove(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_Blur1.GetAddressOf()));
}

void BlurFilter::BuildDescriptor() {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = m_Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_Device->CreateShaderResourceView(m_Blur0.Get(), &srvDesc, m_Blur0CpuSrv);
	m_Device->CreateUnorderedAccessView(m_Blur0.Get(), nullptr, &uavDesc, m_Blur0CpuUav);

	m_Device->CreateShaderResourceView(m_Blur1.Get(), &srvDesc, m_Blur1CpuSrv);
	m_Device->CreateUnorderedAccessView(m_Blur1.Get(), nullptr, &uavDesc, m_Blur1CpuUav);
}

void BlurFilter::BuildDescriptor(
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle,
	UINT DescriptorSize) {

	m_Blur0CpuSrv = cpuHandle;
	m_Blur0CpuUav = cpuHandle.Offset(1, DescriptorSize);
	m_Blur1CpuSrv = cpuHandle.Offset(1, DescriptorSize);
	m_Blur1CpuUav = cpuHandle.Offset(1, DescriptorSize);

	m_Blur0GpuSrv = gpuHandle;
	m_Blur0GpuUav = gpuHandle.Offset(1, DescriptorSize);
	m_Blur1GpuSrv = gpuHandle.Offset(1, DescriptorSize);
	m_Blur1GpuUav = gpuHandle.Offset(1, DescriptorSize);

	BuildDescriptor();
}

void BlurFilter::ResizeBuffer(int width, int height) {
	if (width != m_Width || height != m_Height) {

		m_Width = width;
		m_Height = height;

		BuildResource();

		BuildDescriptor();
	}
}
void BlurFilter::Execute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig,
	ID3D12PipelineState* HoriPso, ID3D12PipelineState* VertPso, ID3D12Resource* rtvResource, int blurCount) {

	auto weights = GaussianBlur(2.5f);
	int blurRadius = (int)(0.5f * weights.size());

	cmdList->SetComputeRootSignature(rootSig);
	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	cmdList->ResourceBarrier(1,
		&unmove(CD3DX12_RESOURCE_BARRIER::Transition(rtvResource,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE)));
	cmdList->ResourceBarrier(1,
		&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur0.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)));

	cmdList->CopyResource(m_Blur0.Get(), rtvResource);

	cmdList->ResourceBarrier(1,
		&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ)));
	cmdList->ResourceBarrier(1,
		&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));

	for (int i = 0; i < blurCount; ++i) {
		// Horizontal
		cmdList->SetPipelineState(HoriPso);

		cmdList->SetComputeRootDescriptorTable(1, m_Blur0GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, m_Blur1GpuUav);

		UINT numGroupsX = (UINT)ceilf(m_Width / 256.0f);
		cmdList->Dispatch(numGroupsX, m_Height, 1.0f);

		cmdList->ResourceBarrier(1,
			&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur0.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));
		cmdList->ResourceBarrier(1,
			&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur1.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ)));


		// Vertical
		cmdList->SetPipelineState(VertPso);

		cmdList->SetComputeRootDescriptorTable(1, m_Blur1GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, m_Blur0GpuUav);

		UINT numGroupsY = (UINT)ceilf(m_Height / 256.0f);
		cmdList->Dispatch(m_Width, numGroupsY, 1.0f);

		cmdList->ResourceBarrier(1,
			&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ)));
		cmdList->ResourceBarrier(1,
			&unmove(CD3DX12_RESOURCE_BARRIER::Transition(m_Blur1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)));
	}
}

ID3D12Resource* BlurFilter::Output() {
	return (m_Blur0.Get());
}

template <class _Tp>
_Tp& BlurFilter::unmove(_Tp&& __value) {
	return __value;
}