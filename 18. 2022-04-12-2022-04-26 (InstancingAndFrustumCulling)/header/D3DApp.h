#pragma once
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxcompiler")

#include "Local/DDSTextureLoader.h"
#include "GameTImer.h"
#include "FrameResource.h"
#include "GeometryGenerator.h"
#include "Camera.h"

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <fstream>

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <d3d12shader.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>

using namespace DirectX;

static const int gNumFrameResources = 3;

#define SOURCE_SHADER_FILE_VS (L"Shader_005.hlsl")
#define SOURCE_SHADER_FILE_PS (L"Shader_005.hlsl")

#define THROWFAILEDIF(e, n) \
{ \
	if(FAILED(n)) { \
		std::cout << e << std::endl; \
		throw std::runtime_error(e); \
	} \
} \

enum class RenderLayer : int
{
	Opaque = 0,
	Shadow,
	Reflected,
	Transparency,
	AlphaTested,
	Mirror,
	Count,
};

namespace Mawi1e {
	template <class _Tp>
	_Tp& My_unmove(_Tp&&);

	struct Texture {
		std::string name;
		std::wstring filename;

		Microsoft::WRL::ComPtr<ID3D12Resource> GPUResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> GPUUploader;
	};

	struct RenderItem {
	public:
		RenderItem() = default;

		XMFLOAT4X4 World = VertexBuffer::GetMatrixIdentity4x4();
		XMFLOAT4X4 TexTransform = VertexBuffer::GetMatrixIdentity4x4();

		int NumFramesDirty = gNumFrameResources;
		// UINT InstanceBufferIndex = -1;

		MeshGeometry* Geo = nullptr;
		Material* Mat = nullptr;

		std::vector<InstanceConstants> Instances;
		UINT InstanceCount;
		BoundingBox Bounds;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT	BaseVertexLocation = 0;
	};

	struct D3DSettings {
		GameTimer** gameTimer = nullptr;
		int screenWidth, screenHeight;
		bool vsync, fullscreen, debugMode;
		bool* appPaused = nullptr;
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
		void Update(const GameTimer*);
		void Draw(const GameTimer*);
		void PrintTime();

		LRESULT MessageHandler(HWND, UINT, WPARAM, LPARAM);
		static D3DApp* GetD3DApp();

	private:
		void GetBoundingBoxFromVertex(BoundingBox& bBox, const std::vector<Vertex>& vertices);

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
		void BuildPSO();
		void BuildInstancesTheSkull();

		/** -----------------------------------------------------------------------------------
		[                            Frame Resources & Render Items                           ]
		----------------------------------------------------------------------------------- **/
		void BuildFrameResources();

		void UpdateInstanceBuffers();
		void UpdatePassCB();

		void LoadTexture();
		void BuildRenderItems();
		void BuildDescriptorHeaps();
		void BuildMaterials();

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

		void DrawRenderItems(ID3D12GraphicsCommandList*, const std::vector<RenderItem*>&);

		float GetHillsHeight(float x, float z) const;
		XMFLOAT3 GetHillsNormal(float x, float z) const;

		void UpdateMatetialCBs(const GameTimer*);
		void OnKeyboardInput(const GameTimer*);
		void UpdateWindowTitle(const GameTimer*);

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

		POINT m_LastMousePos;

		/** -----------------------------------------------------------------------------------
		[                                    Frame Resources                                  ]
		----------------------------------------------------------------------------------- **/
		static const int NumFrameResources = gNumFrameResources;
		std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
		FrameResource* m_CurrFrameResource = nullptr;
		int m_CurrFrameResourceIndex = 0;

		/** -----------------------------------------------------------------------------------
		[                                     Render Items                                    ]
		----------------------------------------------------------------------------------- **/
		std::vector<std::unique_ptr<RenderItem>> m_AllRItems;
		std::vector<RenderItem*> m_OpaqueRItems;
		std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
		RenderItem* mWavesRitem = nullptr;

		PassConstants m_MainPassCB;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_DrawArgs;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_Shaders;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

		UINT m_PassCbvOffset;

		bool m_IsWireFrames = true;

		/** -----------------------------------------------------------------------------------
		[                                 Material & Lighting                                 ]
		----------------------------------------------------------------------------------- **/
		std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;

		/** -----------------------------------------------------------------------------------
		[                                        Textures                                     ]
		----------------------------------------------------------------------------------- **/
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap;

		/** -----------------------------------------------------------------------------------
		[                                 CameraAndDynamicIndexing                            ]
		----------------------------------------------------------------------------------- **/
		Camera m_Camera;

		/** -----------------------------------------------------------------------------------
		[                                 InstancingFrustumCulling                            ]
		----------------------------------------------------------------------------------- **/
		UINT m_InstanceCount = 0;
		UINT m_SkullCounts = 0;
		bool m_isFrustumCulling = 0;
		BoundingFrustum m_LocalProjFrustum;

	};
}

LRESULT _stdcall WindowProcedure(HWND, UINT, WPARAM, LPARAM);