#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class TestSimulation {
public:
	TestSimulation();
	~TestSimulation();

	static XMFLOAT4X4 __vectorcall My_MatrixRotationX(FXMMATRIX, float);
	static XMFLOAT4X4 __vectorcall My_MatrixRotationY(FXMMATRIX, float);
	static XMFLOAT4X4 __vectorcall My_MatrixRotationZ(FXMMATRIX, float);

private:

};