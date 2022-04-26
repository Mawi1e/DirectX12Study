#pragma once
#pragma comment(lib, "D3DCompiler")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dx11")
#pragma comment(lib, "dxgi")

#include <string>
#include <unordered_map>

#include <Windows.h>
#include <wrl.h>

#include <d3d11.h>
#include <D3DX11.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace Mawi1e {
	struct Light {
		DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
		float FallOffStart = 1.0f;
		DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
		float FallOffEnd = 10.0f;
		DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
		float SpotPower = 64.0f;
	};

	float Clamp(float, float, float);
	float RandF();
	float RandF(float, float);
	int Rand(int, int);

	struct SubMeshGeometry {
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;

		DirectX::BoundingBox Bounds;
	};

	struct MeshGeometry {
		std::string Name;

		Microsoft::WRL::ComPtr<ID3DBlob> CPUVertexBuffer;
		Microsoft::WRL::ComPtr<ID3DBlob> CPUIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D12Resource> GPUVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> GPUIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D12Resource> GPUVertexUploader;
		Microsoft::WRL::ComPtr<ID3D12Resource> GPUIndexUploader;

		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;

		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;

		std::unordered_map<std::string, SubMeshGeometry> DrawArgs;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const {
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
			vertexBufferView.BufferLocation = GPUVertexBuffer->GetGPUVirtualAddress();
			vertexBufferView.SizeInBytes = VertexBufferByteSize;
			vertexBufferView.StrideInBytes = VertexByteStride;

			return vertexBufferView;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const {
			D3D12_INDEX_BUFFER_VIEW indexBufferView;
			indexBufferView.BufferLocation = GPUIndexBuffer->GetGPUVirtualAddress();
			indexBufferView.Format = IndexFormat;
			indexBufferView.SizeInBytes = IndexBufferByteSize;

			return indexBufferView;
		}
	};

	class VertexBuffer {
	public:
		VertexBuffer();
		VertexBuffer(const VertexBuffer&);
		~VertexBuffer();

		static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device*, ID3D12GraphicsCommandList*,
			const void*, UINT64, Microsoft::WRL::ComPtr<ID3D12Resource>&);

		static UINT CalcConstantBufferSize(UINT);

		static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring&, const D3D_SHADER_MACRO*, const std::string&, const std::string&);

		static XMFLOAT4X4 GetMatrixIdentity4x4();

		static DirectX::XMVECTOR SphericalToCartesian(float, float, float);

	private:


	private:


	};

	struct Material {
		std::string Name;
		int MatCBIndex = -1;
		int DiffuseSrvHeapIndex = -1;
		int NormalSrvHeapIndex = -1;
		int NumFramesDirty = 3;

		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;

		DirectX::XMFLOAT4X4 MatTransform = VertexBuffer::GetMatrixIdentity4x4();
	};

	struct MaterialConstants {
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;

		DirectX::XMFLOAT4X4 MatTransform = VertexBuffer::GetMatrixIdentity4x4();

		UINT DiffuseMapIndex;
		UINT pad0;
		UINT pad1;
		UINT pad2;
	};
}