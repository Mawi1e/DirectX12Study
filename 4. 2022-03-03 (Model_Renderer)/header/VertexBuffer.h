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
	float Clamp(float, float, float);

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

	private:


	private:


	};
}