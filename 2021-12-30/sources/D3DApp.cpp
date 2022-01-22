#include "D3DApp.h"

void ErrorMessageBox(const char* error) {
	MessageBoxA(0, error, "##Warning##", MB_OK);
	return;
}

D3DApp::D3DApp() {
	m_Win32App = nullptr;
	return;
}

D3DApp::D3DApp(const D3DApp&) {
	return;
}

D3DApp::~D3DApp() {
	this->Shutdown();

	return;
}

void D3DApp::Intialize() {
	HRESULT result;

	/** --------------------------------------------------------------------
	[                           디버깅 계층 생성                           ]
	-------------------------------------------------------------------- **/

#if defined(_DEBUG) || defined(DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
	}
#endif

	if (m_Win32App) {
		throw std::runtime_error("Error : \"Mawi1e::WIN32AppName::win32App != nullptr\"");
	}
	this->IntializeWindow();

	/** --------------------------------------------------------------------
	[                               장치 생성                              ]
	-------------------------------------------------------------------- **/

	ThrowIfFailed("Error: CreateDXGIFactory4",
		CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)));

	result = D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	if (FAILED(result)) {
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	}

	/** --------------------------------------------------------------------
	[                    울타리 생성과 서술자 크기 얻기                    ]
	-------------------------------------------------------------------- **/

	ThrowIfFailed("Error: ID3D12Device::CreateFence",
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

	m_Rtv = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_Dsv = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_Cbv = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	/** --------------------------------------------------------------------
	[                      4X MSAA 품질 수준 지원 점검                     ]
	-------------------------------------------------------------------- **/

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.NumQualityLevels = 0;

	ThrowIfFailed("Error: ID3D12Device::CheckFeatureSupport",
		m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

	char _tmp_Buffer[0x100];
	ZeroMemory(_tmp_Buffer, sizeof(_tmp_Buffer));
	sprintf_s(_tmp_Buffer, "4X MSAA: %u\n", msQualityLevels.NumQualityLevels);
	OutputDebugStringA(_tmp_Buffer);

	/** --------------------------------------------------------------------
	[                     명령 대기열과 명령 목록 생성                     ]
	-------------------------------------------------------------------- **/

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	memset(&commandQueueDesc, 0x00, sizeof(commandQueueDesc));

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed("Error: ID3D12Device::CreateCommandQueue",
		m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_CommandQueue)));

	ThrowIfFailed("Error: ID3D12Device::CreateCommandAllocator",
		m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));

	ThrowIfFailed("Error: ID3D12Device::CreateCommandList",
		m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));

	m_CommandList->Close();

	/** --------------------------------------------------------------------
	[                       교환 사슬의 서술과 생성                        ]
	-------------------------------------------------------------------- **/

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0x00, sizeof(DXGI_SWAP_CHAIN_DESC));

	this->m_SwapChain.Reset();

	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = m_Win32App->GetWidth();
	swapChainDesc.BufferDesc.Height = m_Win32App->GetHeight();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.OutputWindow = m_Win32App->GetHwnd();

	swapChainDesc.SampleDesc.Count = (false ? 4 : 1);
	swapChainDesc.SampleDesc.Quality = (false ? (msQualityLevels.NumQualityLevels - 1) : 0);

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = (m_Win32App->MAWI1E_Fullscreen ? FALSE : TRUE);

	ThrowIfFailed("Error: IDXGIFactory4::CreateSwapChain",
		this->m_Factory->CreateSwapChain(this->m_CommandQueue.Get(),
			&swapChainDesc, this->m_SwapChain.GetAddressOf()));

	/** --------------------------------------------------------------------
	[                            서술자 힙 생성                            ]
	-------------------------------------------------------------------- **/

	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.NumDescriptors = 2;
	RtvHeapDesc.NodeMask = 0;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed("Error: ID3D12Device::CreateDescriptorHeap",
		this->m_Device->CreateDescriptorHeap(&RtvHeapDesc,
			IID_PPV_ARGS(this->m_RtvDescHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.NumDescriptors = 1;
	DsvHeapDesc.NodeMask = 0;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed("Error: ID3D12Device::CreateDescriptorHeap",
		this->m_Device->CreateDescriptorHeap(&DsvHeapDesc,
			IID_PPV_ARGS(this->m_DsvDescHeap.GetAddressOf())));

	/** --------------------------------------------------------------------
	[                        렌더 대상 뷰(RTV) 생성                        ]
	-------------------------------------------------------------------- **/

	


	return;
}

void D3DApp::Shutdown() {
	if (this->m_SwapChain) {
		this->m_SwapChain->SetFullscreenState(0, 0);
	}

	this->ShutdownWindow();

	return;
}

void D3DApp::Render() {
	if (m_Win32App == nullptr) {
		throw std::runtime_error("Error: \"Mawi1e::WIN32AppName::win32App == nullptr\"");
	}

	MSG msg;
	memset(&msg, 0x00, sizeof(MSG));

	for (;;) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			break;
		}

		if (PeekMessage(&msg, m_Win32App->GetHwnd(), 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			break;
		}
	}

	return;
}

void D3DApp::IntializeWindow() {
	m_Win32App = new WIN32App;
	if (m_Win32App == nullptr) {
		throw std::runtime_error("Error: \"Mawi1e::WIN32AppName::win32App == nullptr\"");
	}

	m_Win32App->Intialize(1920, 1080, m_Win32App->MAWI1E_Fullscreen, m_Win32App->MAWI1E_Vsync);

	return;
}

void D3DApp::ShutdownWindow() {
	if (m_Win32App) {
		m_Win32App->Shutdown();
		delete m_Win32App;
		m_Win32App = nullptr;
	}

	return;
}
