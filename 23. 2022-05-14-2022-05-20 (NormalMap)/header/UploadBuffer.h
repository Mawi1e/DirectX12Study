#pragma once

#include "VertexBuffer.h"

template<class T>
class UploadBuffer {
public:
	UploadBuffer(ID3D12Device* device, UINT elementNumber, bool isConstantBuffer) : m_IsConstantBuffer(isConstantBuffer) {
		m_ElementByteSize = sizeof(T);

		if (isConstantBuffer) {
			m_ElementByteSize = Mawi1e::VertexBuffer::CalcConstantBufferSize(sizeof(T));
		}

		auto _a = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto _b = CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementNumber);
		device->CreateCommittedResource(&_a,
			D3D12_HEAP_FLAG_NONE, &_b,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_UploadBuffer.GetAddressOf()));

		m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData));
	}

	UploadBuffer(const UploadBuffer&) = delete;
	UploadBuffer& operator=(const UploadBuffer&) = delete;

	~UploadBuffer() {
		if (m_UploadBuffer != nullptr) {
			m_UploadBuffer->Unmap(0, nullptr);
			m_UploadBuffer = nullptr;
			m_MappedData = nullptr;
		}
	}

	ID3D12Resource* Resource() const {
		return m_UploadBuffer.Get();
	}

	void CopyData(UINT elementIndex, const T& data) {
		memcpy(&m_MappedData[m_ElementByteSize * elementIndex], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer = nullptr;

	BYTE* m_MappedData;
	bool m_IsConstantBuffer;
	UINT m_ElementByteSize;

};