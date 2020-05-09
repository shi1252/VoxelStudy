#pragma once
#include "Defined.h"
#include "AlignedAllocationPolicy.h"

class CameraClass : public AlignedAllocationPolicy<16>
{
public:
	CameraClass();
	CameraClass(const CameraClass&) = delete;
	~CameraClass() = default;

	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	XMFLOAT3 forward();

	void Render();
	void GetViewMatrix(XMMATRIX& view);

private:
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMMATRIX viewMat;
};