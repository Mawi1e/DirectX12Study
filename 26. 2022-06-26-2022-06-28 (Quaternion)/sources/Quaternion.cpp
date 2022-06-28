#include "Quaternion.h"

float BoneAnimation::GetStartTime() const
{
	return keyFrames.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
	float f = keyFrames.back().TimePos;
	return f;
}

void BoneAnimation::Interpolate(float dt, DirectX::XMFLOAT4X4& M) const
{
	if (dt <= keyFrames.front().TimePos)
	{
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR S = XMLoadFloat3(&keyFrames.front().Scale);
		XMVECTOR P = XMLoadFloat3(&keyFrames.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&keyFrames.front().RotationQuat);

		XMMATRIX A = XMMatrixAffineTransformation(S, zero, Q, P);
		XMStoreFloat4x4(&M, A);
	}
	else if (dt >= keyFrames.back().TimePos)
	{
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR S = XMLoadFloat3(&keyFrames.back().Scale);
		XMVECTOR P = XMLoadFloat3(&keyFrames.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&keyFrames.back().RotationQuat);

		XMMATRIX A = XMMatrixAffineTransformation(S, zero, Q, P);
		XMStoreFloat4x4(&M, A);
	}
	else
	{
		for (size_t i = 0; i < keyFrames.size() - 1; ++i)
		{
			if (dt >= keyFrames[i].TimePos && dt <= keyFrames[i + 1].TimePos)
			{
				float t = (dt - keyFrames[i].TimePos) / (keyFrames[i + 1].TimePos - keyFrames[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&keyFrames[i].Scale);
				XMVECTOR p0 = XMLoadFloat3(&keyFrames[i].Translation);
				XMVECTOR q0 = XMLoadFloat4(&keyFrames[i].RotationQuat);

				XMVECTOR s1 = XMLoadFloat3(&keyFrames[i + 1].Scale);
				XMVECTOR p1 = XMLoadFloat3(&keyFrames[i + 1].Translation);
				XMVECTOR q1 = XMLoadFloat4(&keyFrames[i + 1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, t);
				XMVECTOR P = XMVectorLerp(p0, p1, t);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, t);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMMATRIX A = XMMatrixAffineTransformation(S, zero, Q, P);
				XMStoreFloat4x4(&M, A);

				break;
			}
		}
	}
}

QuaternionManager::QuaternionManager()
{
	m_BoneAnimator = {};
	m_AnimateFrame = 0.0f;
}

QuaternionManager::~QuaternionManager()
{
}

void QuaternionManager::Initailize()
{
	XMVECTOR q0 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(30.0f));
	XMVECTOR q1 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 1.0f, 2.0f, 0.0f), XMConvertToRadians(45.0f));
	XMVECTOR q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(-30.0f));
	XMVECTOR q3 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(70.0f));

	m_BoneAnimator.keyFrames.resize(5);
	m_BoneAnimator.keyFrames[0].TimePos = 0.0f;
	m_BoneAnimator.keyFrames[0].Translation = XMFLOAT3(-10.0f, 0.0f, 0.0f);
	m_BoneAnimator.keyFrames[0].Scale = XMFLOAT3(1.25f, 1.25f, 1.25f);
	XMStoreFloat4(&m_BoneAnimator.keyFrames[0].RotationQuat, q0);

	m_BoneAnimator.keyFrames[1].TimePos = 2.0f;
	m_BoneAnimator.keyFrames[1].Translation = XMFLOAT3(-3.0f, 2.0f, 10.0f);
	m_BoneAnimator.keyFrames[1].Scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
	XMStoreFloat4(&m_BoneAnimator.keyFrames[1].RotationQuat, q1);

	m_BoneAnimator.keyFrames[2].TimePos = 4.0f;
	m_BoneAnimator.keyFrames[2].Translation = XMFLOAT3(2.0f, 0.0f, 0.0f);
	m_BoneAnimator.keyFrames[2].Scale = XMFLOAT3(1.25f, 1.25f, 1.25f);
	XMStoreFloat4(&m_BoneAnimator.keyFrames[2].RotationQuat, q2);

	m_BoneAnimator.keyFrames[3].TimePos = 6.0f;
	m_BoneAnimator.keyFrames[3].Translation = XMFLOAT3(-3.0f, 1.0f, -10.0f);
	m_BoneAnimator.keyFrames[3].Scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
	XMStoreFloat4(&m_BoneAnimator.keyFrames[3].RotationQuat, q3);

	m_BoneAnimator.keyFrames[4].TimePos = 8.0f;
	m_BoneAnimator.keyFrames[4].Translation = XMFLOAT3(-10.0f, 0.0f, 0.0f);
	m_BoneAnimator.keyFrames[4].Scale = XMFLOAT3(1.25f, 1.25f, 1.25f);
	XMStoreFloat4(&m_BoneAnimator.keyFrames[4].RotationQuat, q0);
}

void QuaternionManager::Update(float dt, DirectX::XMFLOAT4X4& M)
{
	m_AnimateFrame += dt;

	if (m_AnimateFrame >= m_BoneAnimator.GetEndTime())
	{
		m_AnimateFrame = 0.0f;
	}

	m_BoneAnimator.Interpolate(m_AnimateFrame, M);
}
