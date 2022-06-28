#pragma once

#include "VertexBuffer.h"

struct Keyframe
{
	Keyframe() : TimePos(0.0f), Translation(0.0f, 0.0f, 0.0f), RotationQuat(0.0f, 0.0f, 0.0f, 1.0f), Scale(1.0f, 1.0f, 1.0f) {}
	~Keyframe() {}

	float TimePos;
	DirectX::XMFLOAT3 Translation;
	DirectX::XMFLOAT4 RotationQuat;
	DirectX::XMFLOAT3 Scale;
};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float dt, DirectX::XMFLOAT4X4& M) const;

	std::vector<Keyframe> keyFrames;
};

class QuaternionManager
{
public:
	QuaternionManager();
	QuaternionManager& operator=(const QuaternionManager&) = delete;
	QuaternionManager(const QuaternionManager&) = delete;
	~QuaternionManager();

	void Initailize();
	void Update(float dt, DirectX::XMFLOAT4X4& M);

private:
	BoneAnimation m_BoneAnimator;
	float m_AnimateFrame;

};