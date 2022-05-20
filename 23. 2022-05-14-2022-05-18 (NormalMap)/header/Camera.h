#pragma once

#include "VertexBuffer.h"

#include <DirectXCollision.h>
#include <DirectXMath.h>

using namespace DirectX;

class Camera {
public:
	Camera();
	Camera& operator=(const Camera&) = delete;
	Camera(const Camera&) = delete;
	~Camera();

	XMFLOAT4X4 GetProjectionMatrix() const;
	XMFLOAT4X4 GetViewMatrix() const;
	XMFLOAT3 GetPosition() const;

	void SetPosition(float x, float y, float z);
	void SetLens(float fovY, float ratio, float zn, float zf);
	void Lookat(XMFLOAT3& position, XMFLOAT3& target, XMFLOAT3& up);
	void Lookat(FXMVECTOR position, FXMVECTOR target, FXMVECTOR up);
	void UpdateViewMatrix();

	void Strafe(float d);
	void Walk(float d);
	void Pitch(float d);
	void Roll(float d);
	void RotateY(float d);

private:
	float m_FovY, m_AspectRatio, m_NearZ, m_FarZ;
	float m_NearWindowHeight, m_FarWindowHeight;
	XMFLOAT4X4 m_View = Mawi1e::VertexBuffer::GetMatrixIdentity4x4();
	XMFLOAT4X4 m_Proj = Mawi1e::VertexBuffer::GetMatrixIdentity4x4();
	XMFLOAT3 m_Position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };
	XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
	bool m_ViewDirty = true;

};