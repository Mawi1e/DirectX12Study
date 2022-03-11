#pragma once
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")

#include "VertexBuffer.h"
#include "UploadBuffer.h"

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

#define THROWFAILEDIF(e, n) \
{ \
	if(FAILED(n)) { \
		std::cout << e << std::endl; \
		throw std::runtime_error(e); \
	} \
} \

namespace Mawi1e {
	struct Vertex {
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	struct D3DSettings {
		int screenWidth, screenHeight;
		bool vsync, fullscreen, debugMode;
		HWND hwnd;
	};

	struct UploadObject {
		XMFLOAT4X4 WorldViewProjMatrix = VertexBuffer::GetMatrixIdentity4x4();
	};

	class D3DApp {
	public:
		D3DApp();
		D3DApp(const D3DApp&);
		~D3DApp();

		void Initialize(const D3DSettings&);
		void Shutdown();
		void Update();
		void Draw();

		LRESULT MessageHandler(HWND, UINT, WPARAM, LPARAM);
		static D3DApp* GetD3DApp();

	private:
		void MouseDown(WPARAM, int, int);
		void MouseUp(WPARAM, int, int);
		void MouseMove(WPARAM, int, int);

		float AspectRatio() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle();

		void FlushCommandQueue();
		void ResizeBuffer();

		void EnableDebugLayer();

		void InitializeConsole();

		void LogAdapter();
		void LogOutput();
		void LogModeLists();

		void CreateDevice();
		void Check4xMsaa();

		void CreateFenceAndDescriptorSize();
		void CreateCommandInterface();
		void CreateSwapChain();
		void CreateDescriptorHeap();


		void CreateCbvDescriptorHeap();
		void CreateConstantBufferView();
		void BuildRootSignature();
		void BuildShadersAndInputlayout();
		void BuildBoxGeometry();
		void BuildPSO();

	private:
		static bool m_isD3DSett;
		static D3DApp* m_D3DApp;
		bool m_SizeMinimized = false;
		bool m_SizeMaximized = false;
		bool m_Resizing = false;

		std::unique_ptr<VertexBuffer> m_VertexBuffer;

		UINT64 m_FenceCount = 0;

		const DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static const UINT m_BackBufferCount = 2;
		UINT m_CurrBackBufferIdx = 0;

		const DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		D3DSettings m_d3dSettings;

		UINT m_Numerator, m_Denominator;

		D3D_FEATURE_LEVEL m_MinimumFeatureLevel;
		UINT m_MultisampleQuality;
		bool m_MSQualityState = false;

		UINT m_RtvSize, m_DsvSize, m_CbvSize;

		std::vector<IDXGIAdapter*> m_Adapters;
		std::vector<IDXGIOutput*> m_Outputs;
		std::vector<std::vector<DXGI_MODE_DESC>> m_ModeLists;

		Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_RtvDescriptor[m_BackBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DsvDescriptor;

		D3D12_VIEWPORT m_ViewPort;
		D3D12_RECT m_ScissorRect;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
		std::unique_ptr<UploadBuffer<UploadObject>> m_UploadObj;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

		Microsoft::WRL::ComPtr<ID3DBlob> m_VsByteCode, m_PsByteCode;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElementDesc;
		std::unique_ptr<MeshGeometry> m_ObjMeshGeo;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO;


		XMFLOAT4X4 m_WorldMatrix = VertexBuffer::GetMatrixIdentity4x4();
		XMFLOAT4X4 m_ProjectionMatrix = VertexBuffer::GetMatrixIdentity4x4();
		XMFLOAT4X4 m_ViewMatrix = VertexBuffer::GetMatrixIdentity4x4();

		float m_Theta = 1.5f * XM_PI;
		float m_Phi = XM_PIDIV4;
		float m_Radius = 5.0f;

		POINT m_LastMousePos;
	};
}

LRESULT _stdcall WindowProcedure(HWND, UINT, WPARAM, LPARAM);