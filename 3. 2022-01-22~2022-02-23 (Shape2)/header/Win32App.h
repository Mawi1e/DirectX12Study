#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "D3DApp.h"

namespace Mawi1e {
	struct WindowSettings {
		int screenWidth, screenHeight;
		bool vsync, fullscreen;
	};

	class Win32App {
	public:
		Win32App();
		Win32App(const Win32App&);
		~Win32App();

		void Initialize(const WindowSettings*);
		void Shutdown();
		HWND GetHwnd();

	private:
		LPCWSTR m_Win32ClassName = L"Mawi1e's App";

		HINSTANCE m_Instance;
		HWND m_Hwnd;

		UINT m_ScreenWidth, m_ScreenHeight;
		UINT m_PosX, m_PosY;

	};
}