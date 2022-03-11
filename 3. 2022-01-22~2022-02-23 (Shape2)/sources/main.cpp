#include "ZekrosEngine.h"

int _stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int cmdCurr) {
	try {
		std::unique_ptr<Mawi1e::ZekrosEngine> ZekrosEngine(new Mawi1e::ZekrosEngine);
		ZekrosEngine->Initialize(920, 860);
		ZekrosEngine->Update();
	}
	catch(const std::exception& e) {
		MessageBoxA(0, e.what(), "###WARNING###", MB_ICONWARNING);
	}

	return 0;
}