#include "Camera.h"

Camera::Camera() {
	SetLens(0.25f * XM_PI, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera() {

}

XMFLOAT4X4 Camera::GetProjectionMatrix() const {
	return m_Proj;
}

XMFLOAT4X4 Camera::GetViewMatrix() const {
	return m_View;
}

XMFLOAT3 Camera::GetPosition() const {
	return m_Position;
}

void Camera::SetPosition(float x, float y, float z) {
	m_Position = XMFLOAT3(x, y, z);

	m_ViewDirty = true;
}

void Camera::SetLens(float fovY, float ratio, float zn, float zf) {
	m_FovY = fovY;
	m_AspectRatio = ratio;
	m_NearZ = zn;
	m_FarZ = zf;

	m_NearWindowHeight = 2.0f * m_NearZ * tanf(0.5f * fovY);
	m_FarWindowHeight = 2.0f * m_FarZ * tanf(0.5f * fovY);

	XMMATRIX p = XMMatrixPerspectiveFovLH(fovY, ratio, zn, zf);
	XMStoreFloat4x4(&m_Proj, p);
}

void Camera::Lookat(XMFLOAT3& position, XMFLOAT3& target, XMFLOAT3& up) {
	XMVECTOR P = XMLoadFloat3(&position);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	Lookat(P, T, U);

	m_ViewDirty = true;
}

void Camera::Lookat(FXMVECTOR position, FXMVECTOR target, FXMVECTOR up) {
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, position));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(up, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&m_Position, position);
	XMStoreFloat3(&m_Look, L);
	XMStoreFloat3(&m_Right, R);
	XMStoreFloat3(&m_Up, U);

	m_ViewDirty = true;
}

void Camera::UpdateViewMatrix() {
	if (m_ViewDirty) {
		XMVECTOR P = XMLoadFloat3(&m_Position);
		XMVECTOR L = XMLoadFloat3(&m_Look);
		XMVECTOR R = XMLoadFloat3(&m_Right);
		XMVECTOR U = XMLoadFloat3(&m_Up);

		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));
		R = XMVector3Cross(U, L);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&m_Position, P);
		XMStoreFloat3(&m_Look, L);
		XMStoreFloat3(&m_Right, R);
		XMStoreFloat3(&m_Up, U);

		m_View(0, 0) = m_Right.x; m_View(1, 0) = m_Right.y; m_View(2, 0) = m_Right.z; m_View(3, 0) = x;
		m_View(0, 1) = m_Up.x; m_View(1, 1) = m_Up.y; m_View(2, 1) = m_Up.z; m_View(3, 1) = y;
		m_View(0, 2) = m_Look.x; m_View(1, 2) = m_Look.y; m_View(2, 2) = m_Look.z; m_View(3, 2) = z;
		m_View(0, 3) = 0.0f; m_View(1, 3) = 0.0f; m_View(2, 3) = 0.0f; m_View(3, 3) = 1.0f;

		m_ViewDirty = false;
	}
}

void Camera::Strafe(float d) {
	XMVECTOR R = XMLoadFloat3(&m_Right);
	XMVECTOR P = XMLoadFloat3(&m_Position);
	XMVECTOR D = XMVectorReplicate(d);

	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(D, R, P));

	m_ViewDirty = true;
}

void Camera::Walk(float d) {
	XMVECTOR L = XMLoadFloat3(&m_Look);
	XMVECTOR P = XMLoadFloat3(&m_Position);
	XMVECTOR D = XMVectorReplicate(d);

	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(D, L, P));

	m_ViewDirty = true;
}

void Camera::Pitch(float d) {
	XMVECTOR R = XMLoadFloat3(&m_Right);
	XMVECTOR U = XMLoadFloat3(&m_Up);
	XMVECTOR L = XMLoadFloat3(&m_Look);
	XMMATRIX AXIS = XMMatrixRotationAxis(R, d);

	XMStoreFloat3(&m_Look, XMVector3TransformNormal(L, AXIS));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(U, AXIS));

	m_ViewDirty = true;
}

void Camera::Roll(float d) {
	XMVECTOR R = XMLoadFloat3(&m_Right);
	XMVECTOR U = XMLoadFloat3(&m_Up);
	XMVECTOR L = XMLoadFloat3(&m_Look);
	XMMATRIX AXIS = XMMatrixRotationAxis(L, d);

	XMStoreFloat3(&m_Right, XMVector3TransformNormal(R, AXIS));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(U, AXIS));

	m_ViewDirty = true;
}

void Camera::RotateY(float d) {
	XMVECTOR R = XMLoadFloat3(&m_Right);
	XMVECTOR U = XMLoadFloat3(&m_Up);
	XMVECTOR L = XMLoadFloat3(&m_Look);
	XMMATRIX ROT = XMMatrixRotationY(d);

	XMStoreFloat3(&m_Look, XMVector3TransformNormal(L, ROT));
	XMStoreFloat3(&m_Up, XMVector3TransformNormal(U, ROT));
	XMStoreFloat3(&m_Right, XMVector3TransformNormal(R, ROT));

	m_ViewDirty = true;
}
