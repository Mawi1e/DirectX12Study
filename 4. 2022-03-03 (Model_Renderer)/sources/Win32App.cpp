#include "Win32App.h"

namespace Mawi1e {
	Win32App::Win32App() {
	}

	Win32App::Win32App(const Win32App&) {
	}

	Win32App::~Win32App() {
		Shutdown();
	}

	void Win32App::Initialize(const WindowSettings* windowSettings) {
		DEVMODE devMode;
		WNDCLASSEX wnd;

		m_Instance = GetModuleHandle(0);

		wnd.cbClsExtra = 0;
		wnd.cbSize = sizeof(WNDCLASSEX);
		wnd.cbWndExtra = 0;
		wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wnd.hCursor = LoadCursor(m_Instance, IDC_ARROW);
		wnd.hIcon = LoadIcon(m_Instance, IDI_WINLOGO);
		wnd.hIconSm = wnd.hIcon;
		wnd.hInstance = m_Instance;
		wnd.lpfnWndProc = WindowProcedure;
		wnd.lpszClassName = m_Win32ClassName;
		wnd.lpszMenuName = 0;
		wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		RegisterClassEx(&wnd);

		m_ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		m_ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		if (windowSettings->fullscreen) {
			memset(&devMode, 0x00, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);
			devMode.dmBitsPerPel = 32;
			devMode.dmPelsWidth = m_ScreenWidth;
			devMode.dmPelsHeight = m_ScreenHeight;
			devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			m_PosX = m_PosY = 0;
			ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
		}
		else {
			m_PosX = (UINT)((m_ScreenWidth - windowSettings->screenWidth) / 2);
			m_PosY = (UINT)((m_ScreenHeight - windowSettings->screenHeight) / 2);

			m_ScreenWidth = windowSettings->screenWidth;
			m_ScreenHeight = windowSettings->screenHeight;
		}

		m_Hwnd = CreateWindowEx(0, m_Win32ClassName, m_Win32ClassName,
			WS_VISIBLE | WS_OVERLAPPEDWINDOW, m_PosX, m_PosY, m_ScreenWidth, m_ScreenHeight,
			0, 0, m_Instance, 0);

		ShowWindow(m_Hwnd, SW_SHOW);
		SetForegroundWindow(m_Hwnd);
		SetFocus(m_Hwnd);
		UpdateWindow(m_Hwnd);

		if (windowSettings->fullscreen) ShowCursor(FALSE);
	}

	void Win32App::Shutdown() {
		ChangeDisplaySettings(0, 0);
		ShowCursor(TRUE);
	}

	HWND Win32App::GetHwnd() {
		return m_Hwnd;
	}
}