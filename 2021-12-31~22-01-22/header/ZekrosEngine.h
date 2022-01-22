#pragma once
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#include "Win32App.h"

#include <Windows.h>

#include <iostream>
#include <memory>

namespace Mawi1e {
	class ZekrosEngine {
	public:
		ZekrosEngine();
		ZekrosEngine(const ZekrosEngine&);
		~ZekrosEngine();

		void Initialize(int, int);
		void Update();

	private:


	private:
		std::unique_ptr<Win32App> m_Win32App;
		std::unique_ptr<D3DApp> m_D3DApp;

	};
}