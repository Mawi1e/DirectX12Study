#pragma once

#include "UploadBuffer.h"
#include <memory>

namespace Mawi1e {
	struct ObjConstants {
		DirectX::XMFLOAT4X4 World = VertexBuffer::GetMatrixIdentity4x4();
	};

	struct PassConstants {
		DirectX::XMFLOAT4X4 View = VertexBuffer::GetMatrixIdentity4x4();
		DirectX::XMFLOAT4X4 InvView = VertexBuffer::GetMatrixIdentity4x4();
		DirectX::XMFLOAT4X4 Proj = VertexBuffer::GetMatrixIdentity4x4();
		DirectX::XMFLOAT4X4 InvProj = VertexBuffer::GetMatrixIdentity4x4();
		DirectX::XMFLOAT4X4 ViewProj = VertexBuffer::GetMatrixIdentity4x4();
		DirectX::XMFLOAT4X4 InvViewProj = VertexBuffer::GetMatrixIdentity4x4();

		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float padding001;

		DirectX::XMFLOAT2 RTSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRTSize = { 0.0f, 0.0f };

		float NearZ = 0.0f;
		float FarZ = 0.0f;

		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT4 Color;
	};

	class FrameResource {
	public:
		FrameResource(ID3D12Device*, UINT, UINT);
		FrameResource(const FrameResource&) = delete;
		FrameResource operator=(const FrameResource&) = delete;
		~FrameResource();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;

		std::unique_ptr<UploadBuffer<ObjConstants>> m_ObjCB = nullptr;
		std::unique_ptr<UploadBuffer<PassConstants>> m_PassCB = nullptr;

		UINT64 m_Fence = 0;
	};
}