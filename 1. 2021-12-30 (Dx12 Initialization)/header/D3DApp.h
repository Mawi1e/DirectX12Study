#pragma once
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d12")
#pragma comment(lib,"d3dcompiler.lib")

#define WIN32_LEAN_AND_MEAN

#include "WIN32App.h"

#include <iostream>
#include <exception>
#include <memory>

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>

#define ThrowIfFailed(e, n) { \
if(FAILED(n)) throw std::runtime_error(e); \
} \

#define PtrCleanup(p) if(p) { \
p->Release(); \
delete p; \
p = nullptr; \
} \

void ErrorMessageBox(const char*);

class D3DApp {
public:
	D3DApp();
	D3DApp(const D3DApp&);
	~D3DApp();

	void Intialize();
	void Shutdown();
	void Render();

private:
	void IntializeWindow();
	void ShutdownWindow();

private:
	WIN32App* m_Win32App;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvDescHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvDescHeap;

	UINT m_Rtv, m_Dsv, m_Cbv;

};