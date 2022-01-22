#include "D3DApp.h"

namespace Mawi1e {
	D3DApp::D3DApp() {
		m_D3DApp = this;
		m_VertexBuffer = nullptr;
	}

	D3DApp::D3DApp(const D3DApp&) {
	}

	D3DApp::~D3DApp() {
		Shutdown();
	}

	void D3DApp::Initialize(const D3DSettings& d3dSettings) {
		m_d3dSettings = d3dSettings;

#if defined(DEBUG) || defined(_DEBUG)
		EnableDebugLayer();
#endif

		THROWFAILEDIF("@@@ Error: CreateDXGIFactory1",
			CreateDXGIFactory1(IID_PPV_ARGS(m_Factory.GetAddressOf())));

		if (d3dSettings.debugMode) {
			InitializeConsole();

			LogAdapter();
			LogOutput();
			LogModeLists();
		}

		CreateDevice();
		Check4xMsaa();

		CreateFenceAndDescriptorSize();
		CreateCommandInterface();
		CreateSwapChain();
		CreateDescriptorHeap();

		ResizeBuffer();

		/* --------------------------------------------------------------------------------------
		[                               루트서명&그래픽파이프라인                               ]
		-------------------------------------------------------------------------------------- */

		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

		CreateCbvDescriptorHeap();
		CreateConstantBufferView();
		BuildRootSignature();
		BuildShadersAndInputlayout();
		BuildBoxGeometry();
		BuildPSO();

		m_CommandList->Close();
		ID3D12CommandList* cmdList[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

		FlushCommandQueue();
		//m_isD3DSett = true;
	}

	void D3DApp::Shutdown() {
		if (m_SwapChain != nullptr) {
			FlushCommandQueue();
		}
	}

	float D3DApp::AspectRatio() const {
		return static_cast<float>(m_d3dSettings.screenWidth) / m_d3dSettings.screenHeight;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::GetRtvHandle() {
		auto a = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrBackBufferIdx, m_RtvSize);
		return a;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::GetDsvHandle() {
		return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void D3DApp::FlushCommandQueue() {
		++m_FenceCount;

		THROWFAILEDIF("@@@ Error: ID3D12CommandQueue::Signal",
			m_CommandQueue->Signal(m_Fence.Get(), m_FenceCount));

		if (m_Fence->GetCompletedValue() < m_FenceCount) {
			HANDLE hEvent = CreateEventEx(0, 0, 0, EVENT_ALL_ACCESS);

			THROWFAILEDIF("@@@ Error: ID3D12Fence::SetEventOnCompletion",
				m_Fence->SetEventOnCompletion(m_FenceCount, hEvent));

			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);
		}
	}

	void D3DApp::Update() {
		float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
		float y = m_Radius * cosf(m_Phi);
		float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);

		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&m_ViewMatrix, view);

		XMMATRIX world = XMLoadFloat4x4(&m_WorldMatrix);
		XMMATRIX projection = XMLoadFloat4x4(&m_ProjectionMatrix);
		XMMATRIX worldViewProj = world * view * projection;

		UploadObject constantBuffer;
		XMStoreFloat4x4(&constantBuffer.WorldViewProjMatrix, XMMatrixTranspose(worldViewProj));

		m_UploadObj->CopyData(0, constantBuffer);
	}

	void D3DApp::Draw() {
		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), m_PSO.Get());

		D3D12_RESOURCE_BARRIER resourceBarrier_1 =
			CD3DX12_RESOURCE_BARRIER::Transition(m_RtvDescriptor[m_CurrBackBufferIdx].Get(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->ResourceBarrier(1, &resourceBarrier_1);

		m_CommandList->RSSetViewports(1, &m_ViewPort);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		auto rtv = GetRtvHandle();
		auto dsv = GetDsvHandle();
		FLOAT color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

		m_CommandList->ClearRenderTargetView(rtv, color, 0, nullptr);
		m_CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		m_CommandList->OMSetRenderTargets(1, &rtv, true, &dsv);

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

		auto _1 = m_ObjMeshGeo->VertexBufferView();
		auto _2 = m_ObjMeshGeo->IndexBufferView();
		m_CommandList->IASetVertexBuffers(0, 1, &_1);
		m_CommandList->IASetIndexBuffer(&_2);
		m_CommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_CommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

		m_CommandList->DrawIndexedInstanced(m_ObjMeshGeo->DrawArgs["box"].IndexCount,
			1, 0, 0, 0);

		D3D12_RESOURCE_BARRIER resourceBarrier_2 =
			CD3DX12_RESOURCE_BARRIER::Transition(m_RtvDescriptor[m_CurrBackBufferIdx].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_CommandList->ResourceBarrier(1, &resourceBarrier_2);

		m_CommandList->Close();

		ID3D12CommandList* commandlists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(commandlists), commandlists);

		m_SwapChain->Present(0, 0);
		m_CurrBackBufferIdx = (m_CurrBackBufferIdx + 1) % m_BackBufferCount;

		FlushCommandQueue();
	}

	void D3DApp::ResizeBuffer() {
		FlushCommandQueue();

		m_CommandList->Reset(m_CommandAllocator.Get(), 0);

		for (UINT i = 0; i < m_BackBufferCount; ++i) {
			m_RtvDescriptor[i].Reset();
		}
		m_DsvDescriptor.Reset();

		m_SwapChain->ResizeBuffers(m_BackBufferCount,
			m_d3dSettings.screenWidth, m_d3dSettings.screenHeight,
			m_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		m_CurrBackBufferIdx = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < m_BackBufferCount; ++i) {
			m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RtvDescriptor[i].GetAddressOf()));
			m_Device->CreateRenderTargetView(m_RtvDescriptor[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvSize);
		}

		D3D12_RESOURCE_DESC dsvResourceDesc;
		dsvResourceDesc.Alignment = 0;
		dsvResourceDesc.DepthOrArraySize = 1;
		dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dsvResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		dsvResourceDesc.Height = m_d3dSettings.screenHeight;
		dsvResourceDesc.Width = m_d3dSettings.screenWidth;
		dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsvResourceDesc.MipLevels = 1;
		dsvResourceDesc.SampleDesc.Count = m_MSQualityState ? 4 : 1;
		dsvResourceDesc.SampleDesc.Quality = m_MSQualityState ? (m_MultisampleQuality - 1) : 0;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = m_DepthStencilFormat;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
			&dsvResourceDesc, D3D12_RESOURCE_STATE_COMMON,
			&clearValue, IID_PPV_ARGS(m_DsvDescriptor.GetAddressOf()));

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
		depthStencilDesc.Texture2D.MipSlice = 0;
		depthStencilDesc.Format = m_DepthStencilFormat;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		m_Device->CreateDepthStencilView(m_DsvDescriptor.Get(), &depthStencilDesc,m_DsvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DsvDescriptor.Get(), D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_CommandList->ResourceBarrier(1, &barrier);

		m_CommandList->Close();

		ID3D12CommandList* cmdList[] = { m_CommandList.Get()};
		m_CommandQueue->ExecuteCommandLists(1, cmdList);

		FlushCommandQueue();

		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.MaxDepth = 1.0f;
		m_ViewPort.Height = (FLOAT)m_d3dSettings.screenHeight;
		m_ViewPort.Width = (FLOAT)m_d3dSettings.screenWidth;

		m_ScissorRect = { 0, 0, m_d3dSettings.screenWidth, m_d3dSettings.screenHeight };

		XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&m_ProjectionMatrix, proj);
	}

	void D3DApp::EnableDebugLayer() {
		Microsoft::WRL::ComPtr<ID3D12Debug> debugLayer;
		THROWFAILEDIF("@@@ Error: D3D12GetDebugInterface",
			D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.GetAddressOf())));
		debugLayer->EnableDebugLayer();
	}

	void D3DApp::InitializeConsole() {
		if (AllocConsole() == 0) {
			throw std::runtime_error("Doesn't Alloc Console.");
		}

		FILE* newConIn = nullptr;
		FILE* newConOut = nullptr;
		FILE* newConErr = nullptr;

		freopen_s(&newConIn, "CONIN$", "r", stdin);
		freopen_s(&newConOut, "CONOUT$", "w", stdout);
		freopen_s(&newConErr, "CONOUT$", "w", stderr);

		std::cin.clear();
		std::cout.clear();
		std::cerr.clear();

		std::wcin.clear();
		std::wcout.clear();
		std::wcerr.clear();
	}

	void D3DApp::LogAdapter() {
		IDXGIAdapter* adapter;
		UINT countOf;

		countOf = 0;
		m_Adapters.clear();

		while (m_Factory->EnumAdapters(countOf, &adapter) != DXGI_ERROR_NOT_FOUND) {
			DXGI_ADAPTER_DESC adapterDesc;

			adapter->GetDesc(&adapterDesc);
			WCHAR* description = adapterDesc.Description;
			const SIZE_T videoMemory = (adapterDesc.DedicatedVideoMemory / (1024 * 1024));

			std::wcout << L"*** Adapter: " << description << ", " << videoMemory << " (Mb)" << std::endl;

			m_Adapters.push_back(adapter);
			++countOf;
		}

		std::sort(m_Adapters.begin(), m_Adapters.end(), [](IDXGIAdapter* ial, IDXGIAdapter* iar) {
			return ial > iar;
		});

		std::wcout << std::endl;
	}

	void D3DApp::LogOutput() {
		IDXGIOutput* output;
		UINT countOf;

		countOf = 0;
		m_Outputs.clear();

		for (SIZE_T i = 0; i < m_Adapters.size(); i++) {
			while (m_Adapters[i]->EnumOutputs(countOf, &output) != DXGI_ERROR_NOT_FOUND) {
				DXGI_OUTPUT_DESC outputDesc;

				output->GetDesc(&outputDesc);
				WCHAR* devicename = outputDesc.DeviceName;

				std::wcout << L"*** Output: " << devicename << std::endl;

				m_Outputs.push_back(output);
				++countOf;
			}
		}

		std::sort(m_Outputs.begin(), m_Outputs.end(), [](IDXGIOutput* iol, IDXGIOutput* ior) {
			return iol > ior;
		});

		std::wcout << std::endl;
	}

	void D3DApp::LogModeLists() {
		UINT pNumModes;
		bool Checked;

		Checked = false;
		pNumModes = 0;
		m_ModeLists.clear();

		for (SIZE_T i = 0; i < m_Outputs.size(); i++) {
			m_ModeLists.push_back({});
			m_Outputs[i]->GetDisplayModeList(m_BackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &pNumModes, 0);
			m_ModeLists[i] = std::vector<DXGI_MODE_DESC>(pNumModes);
			m_Outputs[i]->GetDisplayModeList(m_BackBufferFormat, DXGI_ENUM_MODES_INTERLACED, &pNumModes, &m_ModeLists[i][0]);

			for (SIZE_T j = 0; j < m_ModeLists[i].size(); j++) {
				if (m_ModeLists[i][j].Width == (UINT)m_d3dSettings.screenWidth) {
					if (m_ModeLists[i][j].Height == (UINT)m_d3dSettings.screenHeight && !Checked) {
						m_Numerator = m_ModeLists[i][j].RefreshRate.Numerator;
						m_Denominator = m_ModeLists[i][j].RefreshRate.Denominator;

						Checked = true;
					}
				}

				std::wstring wstr;
				wstr.clear();

				wstr += L"*** ModeList " + std::to_wstring(i) + L'\n';
				wstr += L"Width: " + std::to_wstring(m_ModeLists[i][j].Width) + L'\n';
				wstr += L"Height: " + std::to_wstring(m_ModeLists[i][j].Height) + L'\n';
				wstr += L"Resolution: " + std::to_wstring(m_ModeLists[i][j].RefreshRate.Numerator)
					+ L" / " + std::to_wstring(m_ModeLists[i][j].RefreshRate.Denominator) + L'\n';

				std::wcout << wstr << std::endl;
			}

			std::wcout << std::endl;
		}

		std::wcout << std::endl;
	}

	void D3DApp::CreateDevice() {
		HRESULT hadwareResult =
			D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));
		if (FAILED(hadwareResult)) {
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			THROWFAILEDIF("@@@ Error: IDXGIFactory4::EnumWarpAdapter",
				m_Factory->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.GetAddressOf())));

			THROWFAILEDIF("@@@ Error: D3D12CreateDevice",
				D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(m_Device.GetAddressOf())));
		}
	}

	void D3DApp::Check4xMsaa() {
		D3D_FEATURE_LEVEL features[] = {
			D3D_FEATURE_LEVEL_1_0_CORE,
			D3D_FEATURE_LEVEL_9_1,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_2,
		};

		D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevel;
		featureLevel.NumFeatureLevels = 11;
		featureLevel.pFeatureLevelsRequested = features;
		featureLevel.MaxSupportedFeatureLevel = (D3D_FEATURE_LEVEL)0;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CheckFeatureSupport",
			m_Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, (void*)&featureLevel, sizeof(featureLevel)));

		m_MinimumFeatureLevel = featureLevel.MaxSupportedFeatureLevel;

		std::string st4xMsaa;
		switch (m_MinimumFeatureLevel) {
		case D3D_FEATURE_LEVEL_1_0_CORE: st4xMsaa = "DirectX 1.0 core"; break;
		case D3D_FEATURE_LEVEL_9_1: st4xMsaa = "DirectX 9.1"; break;
		case D3D_FEATURE_LEVEL_9_2: st4xMsaa = "DirectX 9.2"; break;
		case D3D_FEATURE_LEVEL_9_3: st4xMsaa = "DirectX 9.3"; break;
		case D3D_FEATURE_LEVEL_10_0: st4xMsaa = "DirectX 10.0"; break;
		case D3D_FEATURE_LEVEL_10_1: st4xMsaa = "DirectX 10.1"; break;
		case D3D_FEATURE_LEVEL_11_0: st4xMsaa = "DirectX 11.0"; break;
		case D3D_FEATURE_LEVEL_11_1: st4xMsaa = "DirectX 11.1"; break;
		case D3D_FEATURE_LEVEL_12_0: st4xMsaa = "DirectX 12.0"; break;
		case D3D_FEATURE_LEVEL_12_1: st4xMsaa = "DirectX 12.1"; break;
		case D3D_FEATURE_LEVEL_12_2: st4xMsaa = "DirectX 12.2"; break;
		}

		std::cout << "*** 4xMsaa(MinimumFeatureLevel): " << st4xMsaa << '\n';
		std::cout << std::endl;


		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevel;
		msQualityLevel.SampleCount = 4;
		msQualityLevel.NumQualityLevels = 0;
		msQualityLevel.Format = m_BackBufferFormat;
		msQualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CheckFeatureSupport",
			m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, (void*)&msQualityLevel, sizeof(msQualityLevel)));

		m_MultisampleQuality = msQualityLevel.NumQualityLevels;

		std::cout << "*** 4xMsaa(MultisampleQuality): " << m_MultisampleQuality << '\n';
		std::cout << std::endl;
	}

	void D3DApp::CreateFenceAndDescriptorSize() {
		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateFence",
			m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf())));

		m_RtvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void D3DApp::CreateCommandInterface() {
		D3D12_COMMAND_QUEUE_DESC queueDesc;
		memset(&queueDesc, 0x00, sizeof(D3D12_COMMAND_QUEUE_DESC));

		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateCommandAllocator",
			m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateCommandList",
			m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(),
				0, IID_PPV_ARGS(m_CommandList.GetAddressOf())));

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateCommandQueue",
			m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

		m_CommandList->Close();
	}

	void D3DApp::CreateSwapChain() {
		m_SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChainDesc.BufferCount = m_BackBufferCount;

		swapChainDesc.BufferDesc.Width = m_d3dSettings.screenWidth;
		swapChainDesc.BufferDesc.Height = m_d3dSettings.screenHeight;
		swapChainDesc.BufferDesc.Format = m_BackBufferFormat;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = m_d3dSettings.debugMode ? m_Numerator : 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = m_d3dSettings.debugMode ? m_Denominator : 1;

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.OutputWindow = m_d3dSettings.hwnd;

		swapChainDesc.SampleDesc.Count = m_MSQualityState ? 4 : 1;
		swapChainDesc.SampleDesc.Quality = m_MSQualityState ? (m_MultisampleQuality - 1) : 0;

		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = m_d3dSettings.fullscreen ? FALSE : TRUE;

		THROWFAILEDIF("@@@ Error: IDXGIFactory4::CreateSwapChain",
			m_Factory->CreateSwapChain(m_CommandQueue.Get(), &swapChainDesc, m_SwapChain.GetAddressOf()));
	}

	void D3DApp::CreateDescriptorHeap() {
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NodeMask = 0;
		rtvHeapDesc.NumDescriptors = m_BackBufferCount;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateDescriptorHeap",
			m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NodeMask = 0;
		dsvHeapDesc.NumDescriptors = 1;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateDescriptorHeap",
			m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
	}

	void D3DApp::CreateCbvDescriptorHeap() {
		D3D12_DESCRIPTOR_HEAP_DESC cbvDesc;
		cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvDesc.NodeMask = 0;
		cbvDesc.NumDescriptors = 1;
		cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateDescriptorHeap",
			m_Device->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(m_CbvHeap.GetAddressOf())));
	}

	void D3DApp::CreateConstantBufferView() {
		m_UploadObj = std::make_unique<UploadBuffer<UploadObject>>(m_Device.Get(), 1, true);
		UINT cbvByteSize = VertexBuffer::CalcConstantBufferSize(sizeof(UploadObject));

		D3D12_GPU_VIRTUAL_ADDRESS virtualAddress = m_UploadObj->Resource()->GetGPUVirtualAddress();

		UINT cbvElementBegin = 0;
		virtualAddress += cbvElementBegin * cbvByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc;
		ConstantBufferViewDesc.BufferLocation = virtualAddress;
		ConstantBufferViewDesc.SizeInBytes = cbvByteSize;

		m_Device->CreateConstantBufferView(&ConstantBufferViewDesc, m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void D3DApp::BuildRootSignature() {
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

		CD3DX12_ROOT_PARAMETER cbvParameter[1];
		cbvParameter[0].InitAsDescriptorTable(1, &cbvTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1,
			cbvParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> serialRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorMsg = nullptr;
		HRESULT hResult = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialRootSig, &errorMsg);

		if (errorMsg != nullptr) {
			OutputDebugStringA((char*)errorMsg->GetBufferPointer());
		}

		THROWFAILEDIF("@@@ Error: D3D12SerializeRootSignature", hResult);

		hResult = m_Device->CreateRootSignature(0, serialRootSig->GetBufferPointer(),
			serialRootSig->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf()));

		THROWFAILEDIF("@@@ Error: ID3D12Device::CreateRootSignature", hResult);
	}

	void D3DApp::BuildShadersAndInputlayout() {
		m_VsByteCode = VertexBuffer::CompileShader(L"..\\D3DApp003\\VertexShader.hlsl", nullptr, "VS", "vs_5_0");
		m_PsByteCode = VertexBuffer::CompileShader(L"..\\D3DApp003\\VertexShader.hlsl", nullptr, "PS", "ps_5_0");

		m_InputElementDesc = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
	}

	void D3DApp::BuildBoxGeometry() {
		std::array<Vertex, 8> vertices =
		{
			Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }),
			Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }),
			Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }),
			Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }),
			Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) })
		};

		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		UINT vertexBufferByteSize = sizeof(Vertex) * vertices.size();
		UINT indexBufferByteSize = sizeof(std::uint16_t) * indices.size();

		m_ObjMeshGeo = std::make_unique<MeshGeometry>();

		D3DCreateBlob(vertexBufferByteSize, m_ObjMeshGeo->CPUVertexBuffer.GetAddressOf());
		CopyMemory(m_ObjMeshGeo->CPUVertexBuffer->GetBufferPointer(), vertices.data(), vertexBufferByteSize);

		D3DCreateBlob(indexBufferByteSize, m_ObjMeshGeo->CPUIndexBuffer.GetAddressOf());
		CopyMemory(m_ObjMeshGeo->CPUIndexBuffer->GetBufferPointer(), indices.data(), indexBufferByteSize);

		m_ObjMeshGeo->GPUVertexBuffer = VertexBuffer::CreateDefaultBuffer(m_Device.Get(),
			m_CommandList.Get(), vertices.data(), vertexBufferByteSize, m_ObjMeshGeo->GPUVertexUploader);
		m_ObjMeshGeo->GPUIndexBuffer = VertexBuffer::CreateDefaultBuffer(m_Device.Get(),
			m_CommandList.Get(), indices.data(), indexBufferByteSize, m_ObjMeshGeo->GPUIndexUploader);

		m_ObjMeshGeo->Name = "boxGeo";

		m_ObjMeshGeo->VertexBufferByteSize = vertexBufferByteSize;
		m_ObjMeshGeo->VertexByteStride = sizeof(Vertex);

		m_ObjMeshGeo->IndexBufferByteSize = indexBufferByteSize;
		m_ObjMeshGeo->IndexFormat = DXGI_FORMAT_R16_UINT;

		SubMeshGeometry subMeshGeo;
		subMeshGeo.IndexCount = indices.size();
		subMeshGeo.BaseVertexLocation = 0;
		subMeshGeo.StartIndexLocation = 0;

		m_ObjMeshGeo->DrawArgs["box"] = subMeshGeo;
	}

	void D3DApp::BuildPSO() {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
		ZeroMemory(&PSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		PSODesc.DSVFormat = m_DepthStencilFormat;
		PSODesc.InputLayout = { m_InputElementDesc.data(), m_InputElementDesc.size() };
		PSODesc.NumRenderTargets = 1;
		PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PSODesc.pRootSignature = m_RootSignature.Get();
		PSODesc.PS = {
			reinterpret_cast<BYTE*>(m_PsByteCode->GetBufferPointer()),
			m_PsByteCode->GetBufferSize(),
		};
		PSODesc.VS = {
			reinterpret_cast<BYTE*>(m_VsByteCode->GetBufferPointer()),
			m_VsByteCode->GetBufferSize(),
		};
		PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PSODesc.RTVFormats[0] = m_BackBufferFormat;
		PSODesc.SampleDesc.Count = m_MSQualityState ? 4 : 1;
		PSODesc.SampleDesc.Quality = m_MSQualityState ? (m_MultisampleQuality - 1) : 0;
		PSODesc.SampleMask = UINT_MAX;

		THROWFAILEDIF("@@@Error: ID3D12Device::CreateGraphicsPipelineState",
			m_Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(m_PSO.GetAddressOf())));
	}

	LRESULT D3DApp::MessageHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
		case WM_ACTIVATE:
			return 0;
		case WM_SIZE:
			if (m_D3DApp != nullptr) {
				m_d3dSettings.screenWidth = LOWORD(lp);
				m_d3dSettings.screenHeight = HIWORD(lp);

				if (m_Device && m_isD3DSett) {
					if (wp == SIZE_MINIMIZED) {
						m_SizeMinimized = true;
						m_SizeMaximized = false;
					}
					else if (wp == SIZE_MAXIMIZED) {
						m_SizeMinimized = false;
						m_SizeMaximized = true;
						ResizeBuffer();
					}
					else if (wp == SIZE_RESTORED) {
						if (m_SizeMinimized) {
							m_SizeMinimized = false;
							ResizeBuffer();
						}
						if (m_SizeMaximized) {
							m_SizeMaximized = false;
							ResizeBuffer();
						}
					}
					else if (m_Resizing) {

					}
					else {
						ResizeBuffer();
					}
				}
			}
			return 0;
		case WM_ENTERSIZEMOVE:
			m_Resizing = true;
			return 0;
		case WM_EXITSIZEMOVE:
			m_Resizing = false;
			ResizeBuffer();
			return 0;
		case WM_LBUTTONDOWN:
			return 0;
		case WM_RBUTTONDOWN:
			MouseDown(wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			return 0;
		case WM_MBUTTONDOWN:
			return 0;
		case WM_LBUTTONUP:
			return 0;
		case WM_RBUTTONUP:
			MouseUp(wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			return 0;
		case WM_MBUTTONUP:
			return 0;
		case WM_MOUSEMOVE:
			MouseMove(wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
			return 0;
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lp)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lp)->ptMinTrackSize.y = 200;
			return 0;
		case WM_MENUCHAR:
			return MAKELRESULT(0, MNC_CLOSE);
		case WM_KEYUP:
			if (wp == VK_ESCAPE) {
				PostQuitMessage(0);
			}
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

	D3DApp* D3DApp::GetD3DApp() {
		return m_D3DApp;
	}

	void D3DApp::MouseDown(WPARAM btnState, int x, int y) {
		if (m_D3DApp != nullptr) {
			m_LastMousePos.x = x;
			m_LastMousePos.y = y;

			SetCapture(m_d3dSettings.hwnd);
		}
	}

	void D3DApp::MouseUp(WPARAM btnState, int x, int y) {
		if (m_D3DApp != nullptr) {
			ReleaseCapture();
		}
	}

	void D3DApp::MouseMove(WPARAM btnState, int x, int y) {
		if ((btnState & MK_LBUTTON) != 0) {
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

			m_Theta += dx;
			m_Phi += dy;

			m_Phi = Clamp(m_Phi, 0.1f, XM_PI - 0.1f);
		}
		else if ((btnState & MK_RBUTTON) != 0) {
			float dx = 0.005f * static_cast<float>(x - m_LastMousePos.x);
			float dy = 0.005f * static_cast<float>(y - m_LastMousePos.y);

			m_Radius += dx - dy;

			m_Radius = Clamp(m_Radius, 5.0f, 15.0f);
		}

		m_LastMousePos.x = x;
		m_LastMousePos.y = y;
	}
}

bool Mawi1e::D3DApp::m_isD3DSett = false;
Mawi1e::D3DApp* Mawi1e::D3DApp::m_D3DApp = nullptr;
LRESULT _stdcall WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	return Mawi1e::D3DApp::GetD3DApp()->MessageHandler(hwnd, msg, wp, lp);
}