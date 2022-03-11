#include "ZekrosEngine.h"

namespace Mawi1e {
	ZekrosEngine::ZekrosEngine() {
		m_Win32App.reset();
		m_D3DApp.reset();
	}

	ZekrosEngine::ZekrosEngine(const ZekrosEngine&) {
	}

	ZekrosEngine::~ZekrosEngine() {
	}

	void ZekrosEngine::Initialize(int width, int height) {
		WindowSettings wndSettings;
		wndSettings.fullscreen = false;
		wndSettings.vsync = true;
		wndSettings.screenWidth = width;
		wndSettings.screenHeight = height;

		m_Win32App = std::make_unique<Win32App>();
		m_Win32App->Initialize(&wndSettings);

		D3DSettings d3dSettings;
		d3dSettings.fullscreen = wndSettings.fullscreen;
		d3dSettings.vsync = wndSettings.vsync;
		d3dSettings.screenWidth = wndSettings.screenWidth;
		d3dSettings.screenHeight = wndSettings.screenHeight;
		d3dSettings.debugMode = true;
		d3dSettings.hwnd = m_Win32App->GetHwnd();

		m_GameTimer = std::make_unique<GameTimer>();
		d3dSettings.gameTimer =  reinterpret_cast<GameTimer**>(&m_GameTimer);
		d3dSettings.appPaused = &m_GamePaused;

		m_D3DApp = std::make_unique<D3DApp>();
		m_D3DApp->Initialize(d3dSettings);
	}

	void ZekrosEngine::Update() {
		MSG msg;
		memset(&msg, 0x00, sizeof(MSG));

		m_GameTimer->Reset();

		for (;;) {
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				DispatchMessage(&msg);
				TranslateMessage(&msg);
			}
			else {
				m_GameTimer->Tick();

				if (!m_GamePaused) {
					m_D3DApp->Update(m_GameTimer.get());
					m_D3DApp->Draw(m_GameTimer.get());
				}
			}

			if (msg.message == WM_QUIT) {
				break;
			}
		}
	}
}