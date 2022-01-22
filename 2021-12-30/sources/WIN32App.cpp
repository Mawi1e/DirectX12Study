#include "WIN32App.h"

WIN32App::WIN32App() {
	return;
}

WIN32App::WIN32App(const WIN32App&) {
	return;
}

WIN32App::~WIN32App() {
	this->Shutdown();

	return;
}

void WIN32App::Intialize(int screenWidth, int screenHeight, bool fullScreen, bool vSync) {
	this->m_ScreenWidth = screenWidth;
	this->m_ScreenHeight = screenHeight;

	HINSTANCE hInst;
	WNDCLASSEX wndClass;
	DEVMODE devMode;

	hInst = GetModuleHandle(0);
	this->m_Instance = hInst;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	wndClass.cbClsExtra = 0;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor = LoadCursor(hInst, IDC_ARROW);
	wndClass.hIcon = LoadIcon(hInst, IDI_WINLOGO);
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.hInstance = hInst;
	wndClass.lpfnWndProc = this->MessageHandler;
	wndClass.lpszClassName = MAWI1E_className;
	wndClass.lpszMenuName = 0;
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	RegisterClassEx(&wndClass);

	if (fullScreen) {
		memset(&devMode, 0x00, sizeof(devMode));
		devMode.dmSize = sizeof(devMode);
		devMode.dmBitsPerPel = 32;
		devMode.dmPelsWidth = screenWidth;
		devMode.dmPelsHeight = screenHeight;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;

		ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
		this->m_PosX = this->m_PosY = 0;
	}
	else {
		this->m_PosX = (int)((screenWidth - this->m_ScreenWidth) * 0.5);
		this->m_PosY = (int)((screenHeight - this->m_ScreenHeight) * 0.5);

		screenWidth = this->m_ScreenWidth;
		screenHeight = this->m_ScreenHeight;
	}

	this->m_Hwnd = CreateWindowEx(WS_EX_APPWINDOW, MAWI1E_className, MAWI1E_className,
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, this->m_PosX, this->m_PosY,
		screenWidth, screenHeight, 0, 0, hInst, 0);
	CheckPtr(this->m_Hwnd);

	ShowWindow(this->m_Hwnd, SW_SHOW);
	SetForegroundWindow(this->m_Hwnd);
	SetFocus(this->m_Hwnd);
	UpdateWindow(this->m_Hwnd);
	ShowCursor(FALSE);

	return;
}

int WIN32App::GetWidth() {
	return (this->m_ScreenWidth);
}

int WIN32App::GetHeight() {
	return (this->m_ScreenHeight);
}

HWND WIN32App::GetHwnd() {
	return (this->m_Hwnd);
}

void WIN32App::Shutdown() {
	if (MAWI1E_Fullscreen) {
		ChangeDisplaySettings(0, 0);
	}

	ShowCursor(TRUE);

	DestroyWindow(this->m_Hwnd);
	UnregisterClassW(MAWI1E_className, this->m_Instance);

	return;
}

LRESULT __stdcall WIN32App::MessageHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}
