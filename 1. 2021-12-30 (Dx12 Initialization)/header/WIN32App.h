#pragma once

#include <Windows.h>

#define CheckPtr(n) \
{ \
	if(n == nullptr) MessageBoxA(0, "", "Error", MB_OK); \
} \

class WIN32App {
public:
	WIN32App();
	WIN32App(const WIN32App&);
	~WIN32App();

	void Intialize(int, int, bool, bool);
	void Shutdown();

	int GetWidth();
	int GetHeight();
	HWND GetHwnd();

	bool MAWI1E_Fullscreen = true;
	bool MAWI1E_Vsync = true;

private:
	static LRESULT __stdcall MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	LPCWSTR MAWI1E_className = L"Mawi1e'sApp";
	HINSTANCE m_Instance;
	HWND m_Hwnd;
	int m_ScreenWidth, m_ScreenHeight;
	int m_PosX, m_PosY;

};