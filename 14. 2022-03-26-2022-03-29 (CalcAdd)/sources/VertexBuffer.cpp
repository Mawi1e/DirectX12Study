#include "VertexBuffer.h"

namespace Mawi1e {
	float Clamp(float target, float min, float max) {
		return (target < min ? min : (target > max ? max : target));
	}

	float RandF() {
		return (float)rand() / (float)RAND_MAX;
	}

	float RandF(float a, float b) {
		return a + (RandF() * (b - a));
	}

	int Rand(int a, int b) {
		return a + (rand() % ((b - a) + 1));
	}

	VertexBuffer::VertexBuffer() {
	}

	VertexBuffer::VertexBuffer(const VertexBuffer&) {
	}

	VertexBuffer::~VertexBuffer() {
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer::CreateDefaultBuffer(ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList, const void* data, UINT64 size,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer) {
		Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer = nullptr;

		D3D12_HEAP_PROPERTIES heap_properties_1 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto BufferSize_1 = CD3DX12_RESOURCE_DESC::Buffer(size);
		device->CreateCommittedResource(&heap_properties_1,
			D3D12_HEAP_FLAG_NONE, &BufferSize_1, D3D12_RESOURCE_STATE_COMMON,
			nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf()));

		D3D12_HEAP_PROPERTIES heap_properties_2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto BufferSize_2 = CD3DX12_RESOURCE_DESC::Buffer(size);
		device->CreateCommittedResource(&heap_properties_2,
			D3D12_HEAP_FLAG_NONE, &BufferSize_2, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(uploadBuffer.GetAddressOf()));


		D3D12_SUBRESOURCE_DATA subResourceData;
		subResourceData.pData = data;
		subResourceData.RowPitch = (LONG_PTR)size;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		auto resourceBarrier_1 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->ResourceBarrier(1, &resourceBarrier_1);

		UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

		auto resourceBarrier_2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &resourceBarrier_2);

		return defaultBuffer;
	}

	UINT VertexBuffer::CalcConstantBufferSize(UINT Size) {
		return (Size + 255) & ~255;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuffer::CompileShader(const std::wstring& fileName,
		const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target) {
		Microsoft::WRL::ComPtr<ID3DBlob> ppCode = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> Error = nullptr;
		UINT flags = 0;


#if defined(DEBUG) || defined(_DEBUG)
		flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hReuslt = D3DX11CompileFromFileW(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entrypoint.c_str(), target.c_str(), flags, 0, nullptr, ppCode.GetAddressOf(), Error.GetAddressOf(), nullptr);

		//D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		//	entrypoint.c_str(), target.c_str(), flags, 0, ppCode.GetAddressOf(), Error.GetAddressOf());

		if (Error != nullptr) {
			OutputDebugStringA((char*)Error->GetBufferPointer());
		}

		if (FAILED(hReuslt)) {
			OutputDebugStringA("@@@Error: D3DX11CompileFromFileW");
		}

		return ppCode;
	}

	XMFLOAT4X4 VertexBuffer::GetMatrixIdentity4x4() {
		static XMFLOAT4X4 l = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		return l;
	}

	DirectX::XMVECTOR VertexBuffer::SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta),
			1.0f);
	}
}