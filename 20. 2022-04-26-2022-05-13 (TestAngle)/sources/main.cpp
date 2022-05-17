#include <iostream>
#include <tuple>
#include <cmath>

#include <DirectXMath.h>
#include <conio.h>

using namespace DirectX;

#define RAD2DEG (180.0f / XM_PI)

std::tuple<float, float, float> GetStoredFloat3(DirectX::FXMVECTOR V) {
	XMFLOAT3 float3;
	XMStoreFloat3(&float3, V);

	return std::make_tuple(float3.x, float3.y, float3.z);
}

float Angle(FXMVECTOR from, FXMVECTOR to) {
	auto [fx, fy, fz] = GetStoredFloat3(from);
	auto [tx, ty, tz] = GetStoredFloat3(to);

	float dot = fx * tx + fy * ty + fz * tz;
	return (acosf(dot) * RAD2DEG);
}

void PrintFixedAngleInfo(FXMVECTOR player_origin, FXMVECTOR forward, FXMVECTOR target_origin) {
	auto [px, py, pz] = GetStoredFloat3(player_origin);
	auto [fx, fy, fz] = GetStoredFloat3(forward);
	auto [tx, ty, tz] = GetStoredFloat3(target_origin);

	std::cout << "플레이어의 위치벡터: " << '(' << px << ',' << py << ',' << pz << ')' << std::endl;
	std::cout << "플레이어가 바라보는 방향벡터: " << '(' << fx << ',' << fy << ',' << fz << ')' << std::endl;
	std::cout << "타겟의 위치벡터: " << '(' << tx << ',' << ty << ',' << tz << ')' << std::endl;

	float angle = Angle(forward, target_origin);
	std::cout << "플레이어가 바라보는 방향과 타겟간의 각도: " << angle << std::endl;

	// <===================================================Fixing=====================================================> //

	auto [cx, cy, cz] = GetStoredFloat3(XMVector3Cross(forward, target_origin));
	std::cout << "외적: " << '(' << cx << ',' << cy << ',' << cz << ')' << std::endl;

	XMMATRIX rot = XMMatrixRotationY(angle);
	XMVECTOR V = XMVector3TransformNormal(forward, rot);

	auto [rx, ry, rz] = GetStoredFloat3(V);
	std::cout << "타겟의 방향에 맞춰 회전된 플레이어가 바라보는 방향벡터: " << '(' << rx << ',' << ry << ',' << rz << ')' << std::endl;

	angle = Angle(V, target_origin);
	std::cout << "갱신된 플레이어가 바라보는 방향과 타겟간의 각도: " << angle << std::endl;
}

int main() {
	XMVECTOR forward =  XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMVECTOR player_origin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR target_origin = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR target_direction = XMVector3Normalize(target_origin - player_origin);

	PrintFixedAngleInfo(player_origin, forward, target_origin);

	_getch();
}