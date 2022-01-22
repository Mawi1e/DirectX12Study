#include "D3DApp.h"

int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpStr, int icmdLine) {
	try {
		std::unique_ptr<D3DApp> d3dApp(new D3DApp);
		d3dApp->Intialize();
		d3dApp->Render();
	}
	catch (const std::exception& e) {
		ErrorMessageBox(e.what());
		return -1;
	}

	return 0;
}