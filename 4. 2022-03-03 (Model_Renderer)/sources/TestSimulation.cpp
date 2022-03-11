#include "TestSimulation.h"

TestSimulation::TestSimulation() {
}

TestSimulation::~TestSimulation() {
}

XMFLOAT4X4 TestSimulation::My_MatrixRotationX(FXMMATRIX mat, float x) {
	XMFLOAT4X4 result;
	XMMATRIX xmat = {
		{ 1, 0, 0, 0 },
		{ 0, cosf(x), sinf(x), 0 },
		{ 0, -sinf(x), cosf(x), 0 },
		{ 0, 0, 0, 1 },
	};

	XMStoreFloat4x4(&result, XMMatrixMultiply(mat, xmat));

	return result;
}

XMFLOAT4X4 TestSimulation::My_MatrixRotationY(FXMMATRIX mat, float y) {
	XMFLOAT4X4 result;
	XMMATRIX ymat = {
		{ cosf(y), 0, -sinf(y), 0 },
		{ 0, 1, 0, 0 },
		{ sinf(y), 0, cosf(y), 0 },
		{ 0, 0, 0, 1 },
	};

	XMStoreFloat4x4(&result, XMMatrixMultiply(mat, ymat));

	return result;
}

XMFLOAT4X4 TestSimulation::My_MatrixRotationZ(FXMMATRIX mat, float z) {
	XMFLOAT4X4 result;
	XMMATRIX zmat = {
		{ cosf(z), sinf(z), 0, 0 },
		{ -sinf(z), cosf(z), 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 },
	};

	XMStoreFloat4x4(&result, XMMatrixMultiply(mat, zmat));

	return result;
}