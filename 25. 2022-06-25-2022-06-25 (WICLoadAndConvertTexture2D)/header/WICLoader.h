#pragma once

#include <Windows.h>
#include <wrl.h>
#include <wincodec.h>

#include <Local/DDSTextureLoader.h>
#include <d3d12.h>
#include <d3dx12.h>


template <class __Tp>
__Tp& unmove(__Tp&& __value);

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

// load and decode image from file
int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);

HRESULT CreateWICTextureFromFile12(_In_ ID3D12Device* device,
	_In_ ID3D12GraphicsCommandList* cmdList,
	_In_z_ const wchar_t* szFileName,
	_Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
	_Out_ Microsoft::WRL::ComPtr<ID3D12Resource>& textureUploadHeap);